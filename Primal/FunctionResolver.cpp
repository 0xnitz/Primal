#include "KernelUtils.hpp"
#include "FunctionResolver.hpp"

namespace FunctionResolver
{

CONST UCHAR PsGetNextProcessSignature[] =
{
    0x48 ,0x89 ,0x5c ,0x24 ,0x08 ,0x48 ,0x89 ,0x6c ,0x24 ,0x18 ,0x48 ,0x89,
	0x74 ,0x24 ,0x20 ,0x57 ,0x41 ,0x54 ,0x41 ,0x55 ,0x41 ,0x56 ,0x41 ,0x57,
	0x48 ,0x83 ,0xec ,0x20 ,0x65 ,0x4c ,0x8b ,0x34 ,0x25 ,0x88 ,0x01 ,0x00,
	0x00 ,0x48 ,0x8b ,0xe9 ,0x33 ,0xf6
};

// TODO: dynamically resolve constant strings here better

NTSTATUS resolve_ps_functions(PDRIVER_OBJECT driver_object)
{
    UNICODE_STRING function_name;
    auto ps_get_next_process = WOBFUSCATE("PsGetNextProcess");
    RtlInitUnicodeString(&function_name, ps_get_next_process);
    const PLDR_DATA_TABLE_ENTRY ntoskrnl_ldr = KernelUtils::get_ntoskrnl_ldr(driver_object);

    PsGetNextProcess = static_cast<PSGETNEXTPROCESS>(find_function_in_module(PsGetNextProcessSignature,
        sizeof(PsGetNextProcessSignature),
        ntoskrnl_ldr->DllBase,
        ntoskrnl_ldr->SizeOfImage));
    if (!PsGetNextProcess) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve PsGetNextProcess\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

	auto ps_get_process_image_file_name = WOBFUSCATE("PsGetProcessImageFileName");
    RtlInitUnicodeString(&function_name, ps_get_process_image_file_name);
    PsGetProcessImageFileName = static_cast<PSGETPROCESSIMAGEFILENAME>(MmGetSystemRoutineAddress(&function_name));
    if (!PsGetProcessImageFileName) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve PsGetProcessImageFileName\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

	auto ps_lookup_process_by_process_id = WOBFUSCATE("PsLookupProcessByProcessId");
	RtlInitUnicodeString(&function_name, ps_lookup_process_by_process_id);
	PsLookupProcessByProcessId = static_cast<PSLOOKUPPROCESSBYPROCESSID>(MmGetSystemRoutineAddress(&function_name));
    if (!PsLookupProcessByProcessId) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve PsLookupProcessByProcessId\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    auto ke_stack_attach_process = WOBFUSCATE("KeStackAttachProcess");
    RtlInitUnicodeString(&function_name, ke_stack_attach_process);
    KeStackAttachProcess = static_cast<KESTACKATTACHPROCESS>(MmGetSystemRoutineAddress(&function_name));
    if (!KeStackAttachProcess) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve KeStackAttachProcess\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    auto ke_unstack_detach_process = WOBFUSCATE("KeUnstackDetachProcess");
    RtlInitUnicodeString(&function_name, ke_unstack_detach_process);
    KeUnstackDetachProcess = static_cast<KEUNSTACKDETACHPROCESS>(MmGetSystemRoutineAddress(&function_name));
    if (!KeUnstackDetachProcess) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve KeUnstackDetachProcess\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    auto zw_allocate_virtual_memory = WOBFUSCATE("ZwAllocateVirtualMemory");
    RtlInitUnicodeString(&function_name, zw_allocate_virtual_memory);
    ZwAllocateVirtualMemory = static_cast<ZWALLOCATEVIRTUALMEMORY>(MmGetSystemRoutineAddress(&function_name));
    if (!ZwAllocateVirtualMemory) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve ZwAllocateVirtualMemory\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    auto ps_get_process_peb = WOBFUSCATE("PsGetProcessPeb");
    RtlInitUnicodeString(&function_name, ps_get_process_peb);
    PsGetProcessPeb = static_cast<PSGETPROCESSPEB>(MmGetSystemRoutineAddress(&function_name));
    if (!PsGetProcessPeb) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve PsGetProcessPeb\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    auto zw_protect_virtual_memory = WOBFUSCATE("ZwProtectVirtualMemory");
    RtlInitUnicodeString(&function_name, zw_protect_virtual_memory);
    ZwProtectVirtualMemory = static_cast<ZWPROTECTVIRTUALMEMORY>(MmGetSystemRoutineAddress(&function_name));
    if (!ZwProtectVirtualMemory) {
        DEBUG_PRINT_OBFUSCATE("Failed to resolve ZwProtectVirtualMemory\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    return STATUS_SUCCESS;
}

PVOID find_function_in_module(IN CONST UCHAR* function_signature,
    IN ULONG function_signature_size,
    IN PVOID ntoskrnl_base_address,
    IN ULONG ntoskrnl_image_size) {

    auto current_base_address = static_cast<UCHAR*>(ntoskrnl_base_address);

    for (ULONG offset = 0; offset < ntoskrnl_image_size; offset++) {
        if (RtlCompareMemory(current_base_address + offset, function_signature, function_signature_size) == function_signature_size) 
        {
            return current_base_address + offset;
        }
    }

    return nullptr;
}

}
