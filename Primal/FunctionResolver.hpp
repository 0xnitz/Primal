#pragma once

#include "DefinesMacros.hpp"

extern PSGETNEXTPROCESS PsGetNextProcess;
extern PSGETPROCESSIMAGEFILENAME PsGetProcessImageFileName;

namespace FunctionResolver
{
	NO_DISCARD NTSTATUS resolve_ps_functions(PDRIVER_OBJECT driver_object);

    NO_DISCARD PVOID find_function_in_module(IN CONST UCHAR* function_signature,
        IN ULONG function_signature_size,
        IN PVOID ntoskrnl_base_address,
        IN ULONG ntoskrnl_image_size);
}