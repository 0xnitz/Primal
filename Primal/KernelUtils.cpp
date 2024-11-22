#include "KernelUtils.hpp"

namespace KernelUtils
{

PLDR_DATA_TABLE_ENTRY get_ntoskrnl_ldr(IN PDRIVER_OBJECT DriverObject)
{
	auto current_entry = static_cast<PLDR_DATA_TABLE_ENTRY>(DriverObject->DriverSection);
	const PLDR_DATA_TABLE_ENTRY first_entry = current_entry;
	constexpr UNICODE_STRING ntoskrnl_name = RTL_CONSTANT_STRING(L"ntoskrnl.exe");

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

}