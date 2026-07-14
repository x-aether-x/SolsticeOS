#pragma once

typedef unsigned long long UINT64;
typedef unsigned long long UINTN;
typedef unsigned int       UINT32;
typedef unsigned short     UINT16;
typedef unsigned char      UINT8;
typedef void               VOID;
typedef unsigned short     CHAR16;
typedef long long          EFI_STATUS;
typedef void* EFI_HANDLE;

#define EFIAPI __attribute__((ms_abi))
#define EFI_SUCCESS 0

typedef enum {
    EfiReservedMemoryType,      // 0: Not usable
    EfiLoaderCode,              // 1: Usable (but holds your bootloader/kernel)
    EfiLoaderData,              // 2: Usable (holds your BootInfo/MemMap)
    EfiBootServicesCode,        // 3: Usable after ExitBootServices
    EfiBootServicesData,        // 4: Usable after ExitBootServices
    EfiRuntimeServicesCode,     // 5: Not usable (keep for UEFI runtime calls like shutdown)
    EfiRuntimeServicesData,     // 6: Not usable 
    EfiConventionalMemory,      // 7: FREE RAM! (This is what you want)
    EfiUnusableMemory,          // 8: Bad RAM
    EfiACPIReclaimMemory,       // 9: Usable after you parse ACPI tables
    EfiACPIMemoryNVS,           // 10: Not usable (ACPI non-volatile)
    EfiMemoryMappedIO,          // 11: Not usable (Device MMIO)
    EfiMemoryMappedIOPortSpace, // 12: Not usable
    EfiPalCode,                 // 13: Not usable
    EfiPersistentMemory,        // 14: Not usable
    EfiMaxMemoryType
} EFI_MEMORY_TYPE;

typedef enum {
    AllocateAnyPages,
    AllocateMaxAddress,
    AllocateAddress
} EFI_ALLOCATE_TYPE;

struct EFI_GUID {
    UINT32 Data1;
    UINT16 Data2;
    UINT16 Data3;
    UINT8  Data4[8];
};

struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
    void* Reset;
    EFI_STATUS (EFIAPI *OutputString)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*, CHAR16*);
    void* TestString;
    void* QueryMode;
    void* SetMode;
    void* SetAttribute;
    EFI_STATUS (EFIAPI *ClearScreen)(struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*);
};

struct EFI_MEMORY_DESCRIPTOR {
    UINT32 Type;
    UINT32 Padding;
    UINT64 PhysicalStart;
    UINT64 VirtualStart;
    UINT64 NumberOfPages;
    UINT64 Attribute;
};

struct EFI_BOOT_SERVICES {
    char Hdr[24];

    void* RaiseTPL;
    void* RestoreTPL;

    EFI_STATUS (EFIAPI *AllocatePages)(EFI_ALLOCATE_TYPE, EFI_MEMORY_TYPE, UINTN, UINT64*);
    void* FreePages;
    EFI_STATUS (EFIAPI *GetMemoryMap)(UINTN*, struct EFI_MEMORY_DESCRIPTOR*, UINTN*, UINTN*, UINT32*);
    EFI_STATUS (EFIAPI *AllocatePool)(EFI_MEMORY_TYPE, UINTN, VOID**);
    void* FreePool;

    void* CreateEvent;
    void* SetTimer;
    void* WaitForEvent;
    void* SignalEvent;
    void* CloseEvent;
    void* CheckEvent;

    void* InstallProtocolInterface;
    void* ReinstallProtocolInterface;
    void* UninstallProtocolInterface;
    EFI_STATUS (EFIAPI *HandleProtocol)(EFI_HANDLE, void*, VOID**);
    void* Reserved;
    void* RegisterProtocolNotify;
    void* LocateHandle;
    void* LocateDevicePath;
    void* InstallConfigurationTable;

    void* LoadImage;
    void* StartImage;
    void* Exit;
    void* UnloadImage;
    EFI_STATUS (EFIAPI *ExitBootServices)(EFI_HANDLE, UINTN);

    void* GetNextMonotonicCount;
    void* Stall;
    void* SetWatchdogTimer;

    void* ConnectController;
    void* DisconnectController;

    void* OpenProtocol;
    void* CloseProtocol;
    void* OpenProtocolInformation;

    void* ProtocolsPerHandle;
    EFI_STATUS (EFIAPI *LocateHandleBuffer)(int, void*, void*, UINTN*, EFI_HANDLE**);
    EFI_STATUS (EFIAPI *LocateProtocol)(void*, void*, VOID**);
    void* InstallMultipleProtocolInterfaces;
    void* UninstallMultipleProtocolInterfaces;

    void* CalculateCrc32;

    void (EFIAPI *CopyMem)(VOID* Destination, VOID* Source, UINTN Length); 
    void* SetMem;
    void* CreateEventEx;
};

struct EFI_SYSTEM_TABLE {
    char Hdr[24];
    CHAR16* FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE StandardErrorHandle;
    void* StdErr;
    void* RuntimeServices;
    struct EFI_BOOT_SERVICES* BootServices;
    UINTN NumberOfTableEntries;
    void* ConfigurationTable;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    void* QueryMode;
    void* SetMode;
    void* Blt;
    struct {
        UINT32 MaxMode;
        UINT32 Mode;
        struct {
            UINT32 Version;
            UINT32 HorizontalResolution;
            UINT32 VerticalResolution;
            int    PixelFormat;
            UINT32 PixelInformation[4];
            UINT32 PixelsPerScanLine;
        } *Info;
        UINTN SizeOfInfo;
        UINT64 FrameBufferBase;
        UINTN FrameBufferSize;
    } *Mode;
};

struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (EFIAPI *OpenVolume)(struct EFI_SIMPLE_FILE_SYSTEM_PROTOCOL*, struct EFI_FILE_PROTOCOL**);
};

struct EFI_FILE_PROTOCOL {
    UINT64 Revision;
    EFI_STATUS (EFIAPI *Open)(struct EFI_FILE_PROTOCOL*, struct EFI_FILE_PROTOCOL**, CHAR16*, UINTN, UINTN);
    EFI_STATUS (EFIAPI *Close)(struct EFI_FILE_PROTOCOL*);
    EFI_STATUS (EFIAPI *Delete)(struct EFI_FILE_PROTOCOL*);
    EFI_STATUS (EFIAPI *Read)(struct EFI_FILE_PROTOCOL*, UINTN*, VOID*);
};

struct EFI_LOADED_IMAGE_PROTOCOL {
    UINT32 Revision;
    EFI_HANDLE ParentHandle;
    struct EFI_SYSTEM_TABLE *SystemTable;
    EFI_HANDLE DeviceHandle;
    void *FilePath;
    void *Reserved;
    UINT32 LoadOptionsSize;
    void *LoadOptions;
    void *ImageBase;
    UINT64 ImageSize;
    EFI_MEMORY_TYPE ImageCodeType;
    EFI_MEMORY_TYPE ImageDataType;
    EFI_STATUS (EFIAPI *Unload)(EFI_HANDLE);
};

#define EFI_FILE_MODE_READ 0x0000000000000001
#define ByProtocol 2