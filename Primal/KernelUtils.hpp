#pragma once

#include "DefinesMacros.hpp"

extern PSGETPROCESSPEB PsGetProcessPeb;
extern KESTACKATTACHPROCESS KeStackAttachProcess;
extern KEUNSTACKDETACHPROCESS KeUnstackDetachProcess;
extern PSLOOKUPPROCESSBYPROCESSID PsLookupProcessByProcessId;

namespace KernelUtils
{

PLDR_DATA_TABLE_ENTRY get_ntoskrnl_ldr(IN PDRIVER_OBJECT DriverObject);

PEPROCESS get_process_of_pid(HANDLE pid);

PPEB get_peb_of_process(PEPROCESS process);

}