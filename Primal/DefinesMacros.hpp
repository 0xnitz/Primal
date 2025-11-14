#pragma once

#include <ntddk.h>

#include "Structs.hpp"
#include "Obfuscation.hpp"

#define NO_DISCARD [[nodiscard]]

#define UNUSED(var) [[maybe_unused]] var

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

#define OBFUSCATE(plaintext) ([]() { \
    constinit static auto obfuscated = ObfuscatedStringA<sizeof(plaintext)>(plaintext); \
    \
    return obfuscated.decrypt(); \
})()

#define WOBFUSCATE(plaintext) ([]() { \
    constinit static auto obfuscated = ObfuscatedStringW<sizeof(plaintext)>(L##plaintext); \
    \
    return obfuscated.decrypt(); \
})()

#define RESOLVE(func_name) [&]() { \
    UNICODE_STRING obfuscated_unicode; \
    RtlInitUnicodeString(&obfuscated_unicode, WOBFUSCATE(#func_name)); \
    \
    return reinterpret_cast<decltype(&func_name)>(MmGetSystemRoutineAddress(&obfuscated_unicode)); \
}()

#ifdef NDEBUG
#define DEBUG_PRINT(format, ...)
#else
// Two macros to support printing DEBUG_PRINT(OBFUSCATE(...)) and variadic
// TODO: obfuscate primal prefix and dybanically resolve DbgPrint
#define DEBUG_PRINT(format, ...) \
    DbgPrint("[Primal] " format, __VA_ARGS__)

#define DEBUG_PRINT_OBFUSCATE(str) \
    DbgPrint("%s %s", OBFUSCATE("[Primal]"), OBFUSCATE(str));
#endif

#define ARCANE_PROCESS_NAME OBFUSCATE("Arcane.exe")

using Address64 = UINT64;
using Byte = UINT8;

extern KSPIN_LOCK PRIMAL_LOCK;

typedef PEPROCESS(*PSGETNEXTPROCESS)(PEPROCESS);
typedef const char* (*PSGETPROCESSIMAGEFILENAME)(PEPROCESS);
typedef NTSTATUS(*PSLOOKUPPROCESSBYPROCESSID)(HANDLE, PEPROCESS*);
typedef VOID(*KESTACKATTACHPROCESS)(PRKPROCESS, PRKAPC_STATE);
typedef VOID(*KEUNSTACKDETACHPROCESS)(PRKAPC_STATE);
typedef NTSYSAPI NTSTATUS(*ZWALLOCATEVIRTUALMEMORY)(HANDLE, PVOID*, ULONG_PTR, PSIZE_T, ULONG, ULONG);
typedef PPEB (*PSGETPROCESSPEB)(IN PEPROCESS Process);
typedef NTSTATUS(*ZWPROTECTVIRTUALMEMORY)(HANDLE ProcessHandle, PVOID* BaseAddress, PSIZE_T RegionSize, ULONG NewProtect, PULONG OldProtect);

extern "C" LIST_ENTRY PsLoadedModuleList;

extern "C" NTKERNELAPI
NTSTATUS
MmCopyVirtualMemory(
    _In_  PEPROCESS FromProcess,
    _In_  PVOID FromAddress,
    _In_  PEPROCESS ToProcess,
    _Out_ PVOID ToAddress,
    _In_  SIZE_T BufferSize,
    _In_  KPROCESSOR_MODE PreviousMode,
    _Out_ PSIZE_T NumberOfBytesCopied
);