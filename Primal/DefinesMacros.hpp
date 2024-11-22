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