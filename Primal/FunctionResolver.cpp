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

NTSTATUS resolve_ps_functions(PDRIVER_OBJECT driver_object)
{
    UNICODE_STRING function_name;

    RtlInitUnicodeString(&function_name, L"PsGetNextProcess");
    const PLDR_DATA_TABLE_ENTRY ntoskrnl_ldr = KernelUtils::get_ntoskrnl_ldr(driver_object);

    PsGetNextProcess = static_cast<PSGETNEXTPROCESS>(find_function_in_module(PsGetNextProcessSignature,
        sizeof(PsGetNextProcessSignature),
        ntoskrnl_ldr->DllBase,
        ntoskrnl_ldr->SizeOfImage));
    if (!PsGetNextProcess) {
        DEBUG_PRINT("Failed to resolve PsGetNextProcess\n");

        return STATUS_PROCEDURE_NOT_FOUND;
    }

    RtlInitUnicodeString(&function_name, L"PsGetProcessImageFileName");
    PsGetProcessImageFileName = static_cast<PSGETPROCESSIMAGEFILENAME>(MmGetSystemRoutineAddress(&function_name));
    if (!PsGetProcessImageFileName) {
        DEBUG_PRINT("Failed to resolve PsGetProcessImageFileName\n");

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
