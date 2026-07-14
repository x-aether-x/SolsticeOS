#include "efi.h"
#include "utils.h"

#define KERNEL_LOCATION 0x100000
#define BOOT_INFO_ADDR  0x9000
#define MEM_MAP_COPY_ADDR 0x80000


// structs for the handoff to the kernel
struct FramebufferInfo {
    UINT64 BaseAddress;
    UINT32 Width;
    UINT32 Height;
    UINT32 Pitch;
};

struct BootInfo {
    FramebufferInfo fb;
    EFI_MEMORY_DESCRIPTOR* mem_map;
    UINTN mem_map_size;
    UINTN mem_desc_size;
};

// protocols we need to steal from UEFI
static EFI_GUID gEfiGraphicsOutputProtocolGuid = {0x9042a9de, 0x23dc, 0x4a38, {0x96, 0xfb, 0x7a, 0xde, 0xd0, 0x80, 0x51, 0x6a}};
static EFI_GUID gEfiSimpleFileSystemProtocolGuid = {0x964e5b22, 0x6459, 0x11d2, {0x8e, 0x39, 0x00, 0xa0, 0xc9, 0x69, 0x72, 0x3b}};

typedef void (*KernelEntry)(BootInfo*);

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) {
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);  
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"[START] Booting sequence started...\n");

    // locate file system handles
    UINTN handle_count;
    EFI_HANDLE *handles;
    SystemTable->BootServices->LocateHandleBuffer(ByProtocol, &gEfiSimpleFileSystemProtocolGuid, NULL, &handle_count, &handles);

    EFI_FILE_PROTOCOL *root = NULL;
    EFI_FILE_PROTOCOL *kernel = NULL;
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* file_system = NULL;

    // find our disk
    for(UINTN i = 0; i < handle_count; i++) {
        if(SystemTable->BootServices->HandleProtocol(handles[i], &gEfiSimpleFileSystemProtocolGuid, (VOID**)&file_system) == EFI_SUCCESS) {
            if(file_system->OpenVolume(file_system, &root) == EFI_SUCCESS) break;
        }
    } 

    if(root == nullptr) {
        SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"Critical: could not find filesystem volume!\n");
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

    // get graphics info so the kernel knows how to draw
    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop;
    SystemTable->BootServices->LocateProtocol(&gEfiGraphicsOutputProtocolGuid, NULL, (VOID**)&gop);
    
    BootInfo *boot_info = (BootInfo*)BOOT_INFO_ADDR;
    if (gop && gop->Mode) {
        boot_info->fb.BaseAddress = gop->Mode->FrameBufferBase;
        boot_info->fb.Width = gop->Mode->Info->HorizontalResolution;
        boot_info->fb.Height = gop->Mode->Info->VerticalResolution;
        boot_info->fb.Pitch = gop->Mode->Info->PixelsPerScanLine * 4;
    }

    // prepare memory map for the kernel PMM
    UINTN map_size = 0;
    UINTN map_key = 0;
    UINTN desc_size = 0;
    UINT32 desc_ver = 0;
    
    // get map size
    SystemTable->BootServices->GetMemoryMap(&map_size, NULL, &map_key, &desc_size, &desc_ver);
    map_size += 4 * desc_size; // buffer
    
    EFI_MEMORY_DESCRIPTOR* map = nullptr;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, map_size, (void**)&map);
    
    // get actual map
    SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &desc_size, &desc_ver);

    boot_info->mem_map = map;
    boot_info->mem_map_size = map_size;
    boot_info->mem_desc_size = desc_size;

    // saving boot map to memory
    SystemTable->BootServices->CopyMem((VOID*)MEM_MAP_COPY_ADDR,(VOID*)boot_info->mem_map,boot_info->mem_map_size);

    // Update the pointer in your BootInfo struct to point to the new safe location
    boot_info->mem_map = (EFI_MEMORY_DESCRIPTOR*)MEM_MAP_COPY_ADDR;

    // jump to kerneel
    if (SystemTable->BootServices->ExitBootServices(ImageHandle, map_key) == EFI_SUCCESS) {
        ((KernelEntry)KERNEL_LOCATION)(boot_info);
    }
    
    return EFI_SUCCESS;
}
