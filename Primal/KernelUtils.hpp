#pragma once

#include "DefinesMacros.hpp"

namespace KernelUtils
{

PLDR_DATA_TABLE_ENTRY get_ntoskrnl_ldr(IN PDRIVER_OBJECT DriverObject);

}