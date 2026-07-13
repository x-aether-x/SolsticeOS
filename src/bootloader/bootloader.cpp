#include "efi.h"

#define KERNEL_LOCATION 0x100000

static EFI_GUID gEfiGraphicsOutputProtocolGuid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
static EFI_GUID gEfiLoadedImageProtocolGuid = {0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
static EFI_GUID gEfiSimpleFileSystemProtocolGuid = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

struct FramebufferInfo {
    UINT64 BaseAddress;
    UINT32 Width;
    UINT32 Height;
    UINT32 Pitch;
};

typedef void (*KernelEntry)(VOID*);

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);  
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"[START] Booting sequence started...b\n");

    // locate loaded image
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&loaded_image);

    // locate file system handles
    UINTN handle_count;
    EFI_HANDLE *handles;
    SystemTable->BootServices->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handle_count, &handles);

    EFI_FILE_PROTOCOL *root = NULL;
    EFI_FILE_PROTOCOL *kernel = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system = NULL;

    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"[DEBUG] Starting handle loop\n");

    UINT64 kernel_addr = KERNEL_LOCATION;
    SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, 512, &kernel_addr); // 2 MB @ 0x100000
    UINT64 info_addr = 0x9000;
    SystemTable->BootServices->AllocatePages(AllocateAddress, EfiLoaderData, 1, &info_addr);

    for(UINTN i = 0; i < handle_count; i++) {
        if(handles[i] == nullptr) continue;

        EFI_STATUS status = SystemTable->BootServices->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&file_system);
        
        if(status == EFI_SUCCESS && file_system != nullptr) {
            if(file_system->OpenVolume(file_system, &root) == EFI_SUCCESS) {
                SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"[DEBUG] Volume opened successfully!\n");
                break;
            }
        }
    } 

    if(root == nullptr) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"Critical: could not find/open simplefilesystem volume!\n");
        while(1);
    }

    // load kernel file
    root->Open(root, &kernel, (CHAR16*)L"kernel.bin", EFI_FILE_MODE_READ, 0);
    UINTN bytes_loaded = 0;
    for (;;) {
        UINTN chunk_size = 0x10000;
        EFI_STATUS status = kernel->Read(kernel, &chunk_size, (VOID*)(KERNEL_LOCATION + bytes_loaded));
        if (status != EFI_SUCCESS || chunk_size == 0) break;
        bytes_loaded += chunk_size;
    }
    kernel->Close(kernel);
    root->Close(root);

    // get graphics info
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    SystemTable->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&gop);
    
    FramebufferInfo *fb_info = (FramebufferInfo*)0x9000;
    fb_info->BaseAddress = (gop && gop->Mode) ? gop->Mode->FrameBufferBase : 0;
    if (gop && gop->Mode && gop->Mode->Info) {
        fb_info->Width = gop->Mode->Info->HorizontalResolution;
        fb_info->Height = gop->Mode->Info->VerticalResolution;
        fb_info->Pitch = gop->Mode->Info->PixelsPerScanLine * 4;
    }

    // prepare memory map and exit boot services
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_ver = 0;
    EFI_MEMORY_DESCRIPTOR* map = nullptr;
    
    // get map size
    SystemTable->BootServices->GetMemoryMap(&map_size, NULL, &map_key, &desc_size, &desc_ver);
    map_size += 2 * desc_size;
    
    // allocate buffer for map
    SystemTable->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&map);
    
    // get map and exit
    if (SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &desc_size, &desc_ver) == EFI_SUCCESS) {
        SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);
    }

    // jump to kernel
    ((KernelEntry)KERNEL_LOCATION)((VOID*)0x9000);
    
    return EFI_SUCCESS;
}