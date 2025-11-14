#include "KernelUtils.hpp"

namespace KernelUtils
{

PLDR_DATA_TABLE_ENTRY get_ntoskrnl_ldr(IN PDRIVER_OBJECT DriverObject)
{
	auto current_entry = static_cast<PLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);
	const PLDR_DATA_TABLE_ENTRY first_entry = current_entry;
	auto ntos_name = WOBFUSCATE("ntoskrnl.exe");
	UNICODE_STRING ntoskrnl_name;
	RtlInitUnicodeString(&ntoskrnl_name, ntos_name);

	while (reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(current_entry->InLoadOrderLinks.Flink) != first_entry)
	{
		if (RtlCompareUnicodeString(&current_entry->BaseDllName, &ntoskrnl_name, TRUE) == 0)
		{
			return current_entry;
		}

		current_entry = reinterpret_cast<PLDR_DATA_TABLE_ENTRY>(current_entry->InLoadOrderLinks.Flink);
	}

	return nullptr;
}

PEPROCESS get_process_of_pid(HANDLE pid)
{
	PEPROCESS process = nullptr;
	if (NT_SUCCESS(PsLookupProcessByProcessId(pid, &process)) && process)
	{
		return process;
	}

	return nullptr;
}

PPEB get_peb_of_process(PEPROCESS process)
{
	KAPC_STATE state;
	PRKPROCESS kprocess = reinterpret_cast<PRKPROCESS>(process);
	KeStackAttachProcess(kprocess, &state);

	PPEB peb = PsGetProcessPeb(process);

	KeUnstackDetachProcess(&state);

	return peb;
}

}