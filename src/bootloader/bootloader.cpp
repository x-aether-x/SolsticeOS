typedef unsigned short CHAR16;
typedef unsigned short UINT16;
typedef unsigned int   UINT32;
typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef void* VOID;
typedef void* EFI_HANDLE;
typedef UINTN          EFI_STATUS;

#define EFI_SUCCESS 0
#define KERNEL_LOCATION 0x100000 
#define EFIAPI __attribute__((ms_abi)) 

// defined memory layout structure for graphics handoff communication
struct FramebufferInfo {
    UINT64 BaseAddress;
    UINT32 Width;
    UINT32 Height;
    UINT32 Pitch;
};

struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;
struct _EFI_BOOT_SERVICES;

typedef struct {
    UINT64 Signature; UINT32 Revision; UINT32 HeaderSize; UINT32 CRC32; UINT32 Reserved;
} EFI_TABLE_HEADER;

typedef struct {
    EFI_TABLE_HEADER Hdr; CHAR16 *FirmwareVendor; UINT32 FirmwareRevision; EFI_HANDLE ConsoleInHandle;
    VOID *ConIn; EFI_HANDLE ConsoleOutHandle; struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *ConOut;
    EFI_HANDLE StandardErrorHandle; VOID *StdErr; VOID *RuntimeServices; struct _EFI_BOOT_SERVICES *BootServices;
    UINTN NumberOfTableEntries; VOID *ConfigurationTable;
} EFI_SYSTEM_TABLE;

typedef struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    VOID *Reset; EFI_STATUS (EFIAPI *OutputString)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This, CHAR16 *String);
    VOID *TestString; VOID *QueryMode; VOID *SetMode; VOID *SetAttribute;
    EFI_STATUS (EFIAPI *ClearScreen)(struct _EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL *This);
} EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL;

typedef struct {
    EFI_HANDLE DeviceHandle; VOID *FilePath; VOID *DeviceSpecificInfo; UINT32 ImageOptionsSize;
    VOID *ImageOptions; VOID *ImageBase; UINT64 ImageSize;
} EFI_LOADED_IMAGE_PROTOCOL;

typedef struct _EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (EFIAPI *Open)(struct _EFI_FILE_PROTOCOL *This, struct _EFI_FILE_PROTOCOL **NewHandle, CHAR16 *FileName, UINT64 OpenMode, UINT64 Attributes);
    EFI_STATUS (EFIAPI *Close)(struct _EFI_FILE_PROTOCOL *This);
    VOID *Delete; EFI_STATUS (EFIAPI *Read)(struct _EFI_FILE_PROTOCOL *This, UINTN *BufferSize, VOID *Buffer);
} EFI_FILE_PROTOCOL;

typedef struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 Revision; EFI_STATUS (EFIAPI *OpenVolume)(struct _EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This, EFI_FILE_PROTOCOL **Root);
} EFI_SIMPLE_FILE_SYSTEM_PROTOCOL;

typedef struct { UINT32 Data1; UINT16 Data2; UINT16 Data3; unsigned char Data4[8]; } EFI_GUID;

typedef struct {
    UINT32 Version;
    UINT32 HorizontalResolution;
    UINT32 VerticalResolution;
    UINT32 PixelFormat;
    UINT32 Reserved[4];
    UINT32 PixelsPerScanLine;
} EFI_GRAPHICS_OUTPUT_MODE_INFORMATION;

typedef struct {
    UINT32 MaxMode; UINT32 Mode; VOID *Info; UINTN SizeOfInfo; UINT64 FrameBufferBase; UINTN FrameBufferSize;
} EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE;

typedef struct _EFI_GRAPHICS_OUTPUT_PROTOCOL {
    VOID *QueryMode; VOID *SetMode; VOID *Blt; EFI_GRAPHICS_OUTPUT_PROTOCOL_MODE *Mode;
} EFI_GRAPHICS_OUTPUT_PROTOCOL;

static EFI_GUID LoadedImageProtocolGuid = { 0x5B1B31A1, 0x9562, 0x11d2, { 0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B } };
static EFI_GUID SimpleFileSystemProtocolGuid = { 0x964E5B22, 0x6459, 0x11d2, { 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B } };
static EFI_GUID GOPGuid = { 0x9042A9DE, 0x23DC, 0x4A38, { 0x96, 0xFB, 0x7A, 0xDE, 0x0D, 0x69, 0x82, 0x47 } };

typedef struct _EFI_BOOT_SERVICES {
    EFI_TABLE_HEADER Hdr; VOID *RaiseTPL; VOID *RestoreTPL; VOID *AllocatePages; VOID *FreePages;
    EFI_STATUS (EFIAPI *GetMemoryMap)(UINTN *MemoryMapSize, VOID *MemoryMap, UINTN *MapKey, UINTN *DescriptorSize, UINT32 *DescriptorVersion);
    VOID *AllocatePool; VOID *FreePool; VOID *CreateEvent; VOID *SetTimer; VOID *WaitForEvent; VOID *SignalEvent;
    VOID *CloseEvent; VOID *CheckEvent; VOID *InstallProtocolInterface; VOID *ReinstallProtocolInterface; VOID *UninstallProtocolInterface;
    EFI_STATUS (EFIAPI *HandleProtocol)(EFI_HANDLE Handle, EFI_GUID *Protocol, VOID **Interface);
    VOID *VoidReserved; VOID *RegisterProtocolNotify;
    EFI_STATUS (EFIAPI *LocateHandleBuffer)(int SearchType, EFI_GUID *Protocol, VOID *SearchKey, UINTN *NoHandles, EFI_HANDLE **Buffer);
    VOID *LocateDevicePath; VOID *InstallConfigurationTable; VOID *LoadImage; VOID *StartImage; VOID *Exit; VOID *UnloadImage;
    EFI_STATUS (EFIAPI *ExitBootServices)(EFI_HANDLE ImageHandle, UINTN MapKey);
    EFI_STATUS (EFIAPI *LocateProtocol)(EFI_GUID *Protocol, VOID *Registration, VOID **Interface);
} EFI_BOOT_SERVICES;

typedef void (*KernelEntry)(VOID*); 

extern "C" EFI_STATUS EFIAPI efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE *SystemTable) { // uefi application entry point
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut); // reset display
    SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"--- SolsticeOS Bootloader Active ---\n"); // debug status ping

    EFI_LOADED_IMAGE_PROTOCOL *loaded_image; // profile container
    SystemTable->BootServices->HandleProtocol(ImageHandle, &LoadedImageProtocolGuid, (VOID**)&loaded_image); // fetch current loader tracking

    UINTN handle_count; EFI_HANDLE *handles; // collect hardware mount handles
    SystemTable->BootServices->LocateHandleBuffer(2, &SimpleFileSystemProtocolGuid, nullptr, &handle_count, &handles); // enumerate disks
    
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *file_system; EFI_FILE_PROTOCOL *root, *kernel; // storage controller interfaces
    for(UINTN i = 0; i < handle_count; i++) { // search interfaces for valid volume
        if(SystemTable->BootServices->HandleProtocol(handles[i], &SimpleFileSystemProtocolGuid, (VOID**)&file_system) == EFI_SUCCESS) break;
    }
    file_system->OpenVolume(file_system, &root); // unlock root volume
    root->Open(root, &kernel, (CHAR16*)L"kernel.bin", 1, 0); // open kernel binary payload

    UINTN bytes_loaded = 0;
    for (;;) {
        UINTN chunk_size = 0x10000;
        EFI_STATUS status = kernel->Read(kernel, &chunk_size, (VOID*)(KERNEL_LOCATION + bytes_loaded));
        if (status != EFI_SUCCESS || chunk_size == 0) break;
        bytes_loaded += chunk_size;
    }
    kernel->Close(kernel); root->Close(root); // clean handles

    EFI_GRAPHICS_OUTPUT_PROTOCOL *gop; // grab graphics interface
    SystemTable->BootServices->LocateProtocol((EFI_GUID*)&GOPGuid, nullptr, (VOID**)&gop); // query video display protocol
    
    FramebufferInfo *fb_info = (FramebufferInfo*)0x9000; // map address to structure interface
    fb_info->BaseAddress = gop && gop->Mode ? gop->Mode->FrameBufferBase : 0;
    if (gop && gop->Mode && gop->Mode->Info) {
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION *mode_info = (EFI_GRAPHICS_OUTPUT_MODE_INFORMATION*)gop->Mode->Info;
        fb_info->Width = mode_info->HorizontalResolution;
        fb_info->Height = mode_info->VerticalResolution;
        fb_info->Pitch = mode_info->PixelsPerScanLine * 4;
    } else {
        fb_info->Width = 1024;
        fb_info->Height = 768;
        fb_info->Pitch = 1024 * 4;
    }

    UINTN map_size = 0, map_key, desc_size; UINT32 desc_ver; // memory map parameters
    SystemTable->BootServices->GetMemoryMap(&map_size, nullptr, &map_key, &desc_size, &desc_ver); // fetch size
    VOID *map = (VOID*)0x60000; // assign map storage location
    SystemTable->BootServices->GetMemoryMap(&map_size, map, &map_key, &desc_size, &desc_ver); // retrieve system map
    SystemTable->BootServices->ExitBootServices(ImageHandle, map_key); // relinquish firmware ownership

    ((KernelEntry)KERNEL_LOCATION)((VOID*)0x9000); // branch to kernel with context argument
    while(1); // safety infinite hang
    return EFI_SUCCESS;
}