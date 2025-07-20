#pragma once

#include <ntddk.h>

#define NO_DISCARD [[nodiscard]]

#define UNUSED(var) [[maybe_unused]] var

#ifdef NDEBUG
#define DEBUG_PRINT(format, ...)
#else
#define DEBUG_PRINT(format, ...) \
    DbgPrint("[Primal] " format, __VA_ARGS__)
#endif

#define ARCANE_PROCESS_NAME "Arcane.exe"

#define LOCK() \
    KIRQL old_irql; \
    KeAcquireSpinLock(&PRIMAL_LOCK, &old_irql);

#define UNLOCK() \
    KeReleaseSpinLock(&PRIMAL_LOCK, old_irql);

#define COMPLETE_REQUEST(irp, status) \
    irp->IoStatus.Status = status; \
    IoCompleteRequest(irp, IO_NO_INCREMENT); \
    \
    return status;

#define COMPLETE_REQUEST_UNLOCK(irp, status) \
    irp->IoStatus.Status = status; \
    IoCompleteRequest(irp, IO_NO_INCREMENT); \
    UNLOCK(); \
    \
    return status;

extern KSPIN_LOCK PRIMAL_LOCK;

typedef PEPROCESS(*PSGETNEXTPROCESS)(PEPROCESS);
typedef const char* (*PSGETPROCESSIMAGEFILENAME)(PEPROCESS);

typedef struct _LDR_DATA_TABLE_ENTRY {
    LIST_ENTRY InLoadOrderLinks;
    LIST_ENTRY InMemoryOrderLinks;
    LIST_ENTRY InInitializationOrderLinks;
    PVOID DllBase;
    PVOID EntryPoint;
    ULONG SizeOfImage;
    UNICODE_STRING FullDllName;
    UNICODE_STRING BaseDllName;
    ULONG Flags;
    USHORT LoadCount;
    USHORT TlsIndex;
    LIST_ENTRY HashLinks;
    ULONG TimeDateStamp;
    PVOID EntryPointActivationContext;
    PVOID PatchInformation;
    LIST_ENTRY ForwarderLinks;
    LIST_ENTRY ServiceTagLinks;
    LIST_ENTRY StaticLinks;
    PVOID ContextInformation;
    ULONG OriginalBase;
    LARGE_INTEGER LoadTime;
} LDR_DATA_TABLE_ENTRY, * PLDR_DATA_TABLE_ENTRY;

typedef struct _KLDR_DATA_TABLE_ENTRY
{
    struct _LIST_ENTRY InLoadOrderLinks;                                    //0x0
    VOID* ExceptionTable;                                                   //0x10
    ULONG ExceptionTableSize;                                               //0x18
    VOID* GpValue;                                                          //0x20
    struct _NON_PAGED_DEBUG_INFO* NonPagedDebugInfo;                        //0x28
    VOID* DllBase;                                                          //0x30
    VOID* EntryPoint;                                                       //0x38
    ULONG SizeOfImage;                                                      //0x40
    struct _UNICODE_STRING FullDllName;                                     //0x48
    struct _UNICODE_STRING BaseDllName;                                     //0x58
    ULONG Flags;                                                            //0x68
    USHORT LoadCount;                                                       //0x6c
    union
    {
        USHORT SignatureLevel : 4;                                            //0x6e
        USHORT SignatureType : 3;                                             //0x6e
        USHORT Frozen : 2;                                                    //0x6e
        USHORT HotPatch : 1;                                                  //0x6e
        USHORT Unused : 6;                                                    //0x6e
        USHORT EntireField;                                                 //0x6e
    } u1;                                                                   //0x6e
    VOID* SectionPointer;                                                   //0x70
    ULONG CheckSum;                                                         //0x78
    ULONG CoverageSectionSize;                                              //0x7c
    VOID* CoverageSection;                                                  //0x80
    VOID* LoadedImports;                                                    //0x88
    union
    {
        VOID* Spare;                                                        //0x90
        struct _KLDR_DATA_TABLE_ENTRY* NtDataTableEntry;                    //0x90
    };
    ULONG SizeOfImageNotRounded;                                            //0x98
    ULONG TimeDateStamp;                                                    //0x9c
} KLDR_DATA_TABLE_ENTRY, * PKLDR_DATA_TABLE_ENTRY;

extern "C" LIST_ENTRY PsLoadedModuleList;

using Address64 = UINT64;