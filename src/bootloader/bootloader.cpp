#include "efi.h" // Your custom minimal header

#define KERNEL_LOCATION 0x100000

extern "C" EFI_GUID gEfiGraphicsOutputProtocolGuid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
extern "C" EFI_GUID gEfiLoadedImageProtocolGuid = {0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
extern "C" EFI_GUID gEfiSimpleFileSystemProtocolGuid = {0x0964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

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

    // get image
    EFI_LOADED_IMAGE_PROTOCOL *loaded_image;
    SystemTable->BootServices->HandleProtocol(ImageHandle, &gEfiLoadedImageProtocolGuid, (VOID**)&loaded_image);

    // locate fs
    UINTN handle_count;
    EFI_HANDLE *handles;
    SystemTable->BootServices->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handle_count, &handles);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system = NULL;
    EFI_FILE_PROTOCOL *root = NULL;
    EFI_FILE_PROTOCOL *kernel = NULL;

    for(UINTN i = 0; i < handle_count; i++) {
        if(SystemTable->BootServices->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&file_system) == EFI_SUCCESS) {
            if(file_system->OpenVolume(file_system, &root) == EFI_SUCCESS) break;
        }
    }

    if (!root) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"CRITICAL: root handle is NULL!\n"); 
        while(1);
    }

    // load
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

    // graphics
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    SystemTable->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&gop);
    
    FramebufferInfo *fb_info = (FramebufferInfo*)0x9000;
    fb_info->BaseAddress = (gop && gop->Mode) ? gop->Mode->FrameBufferBase : 0;
    if (gop && gop->Mode && gop->Mode->Info) {
        fb_info->Width = gop->Mode->Info->HorizontalResolution;
        fb_info->Height = gop->Mode->Info->VerticalResolution;
        fb_info->Pitch = gop->Mode->Info->PixelsPerScanLine * 4;
    }

    // memory map
    UINTN map_size = 0;
    UINTN map_key, desc_size;
    UINT32 desc_ver;
    
    SystemTable->BootServices->GetMemoryMap(&map_size, NULL, &map_key, &desc_size, &desc_ver);
    map_size += 2 * desc_size;
    EFI_MEMORY_DESCRIPTOR *map = NULL;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, map_size, (VOID**)&map); 
    
    SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &desc_size, &desc_ver);
    SystemTable->BootServices->ExitBootServices(ImageHandle, map_key);

    ((KernelEntry)KERNEL_LOCATION)((VOID*)0x9000);
    
    return EFI_SUCCESS;
}