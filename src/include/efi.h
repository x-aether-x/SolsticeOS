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
#define NULL 0

typedef enum { EfiLoaderData } EFI_MEMORY_TYPE;

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
    EFI_STATUS (EFIAPI *AllocatePool)(EFI_MEMORY_TYPE, UINTN, VOID**);
    void* FreePool;
    EFI_STATUS (EFIAPI *LocateHandleBuffer)(int, void*, void*, UINTN*, EFI_HANDLE**);
    EFI_STATUS (EFIAPI *HandleProtocol)(EFI_HANDLE, void*, VOID**);
    void* Stall;
    void* SetWatchdogTimer;
    EFI_STATUS (EFIAPI *LocateProtocol)(void*, void*, VOID**);
    EFI_STATUS (EFIAPI *GetMemoryMap)(UINTN*, struct EFI_MEMORY_DESCRIPTOR*, UINTN*, UINTN*, UINT32*);
    EFI_STATUS (EFIAPI *ExitBootServices)(EFI_HANDLE, UINTN);
};

struct EFI_SYSTEM_TABLE {
    char Hdr[24];
    CHAR16* FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    void* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    struct EFI_BOOT_SERVICES* BootServices;
};

struct EFI_GRAPHICS_OUTPUT_PROTOCOL {
    void* QueryMode;
    void* SetMode;
    struct {
        void* MaxMode;
        void* Mode;
        struct {
            UINT32 MaxResolutionX;
            UINT32 MaxResolutionY;
            UINT32 HorizontalResolution;
            UINT32 VerticalResolution;
            int PixelFormat;
            void* PixelInformation;
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