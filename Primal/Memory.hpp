#pragma once

#include "DefinesMacros.hpp"

namespace Memory
{
	NO_DISCARD NTSTATUS primal_read(PDEVICE_OBJECT DeviceObject, PIRP Irp);

	NO_DISCARD NTSTATUS primal_write(PDEVICE_OBJECT DeviceObject, PIRP Irp);
};