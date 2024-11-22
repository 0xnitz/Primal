#pragma once

#include "DefinesMacros.hpp"

NO_DISCARD NTSTATUS primal_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NO_DISCARD NTSTATUS primal_control(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NO_DISCARD DRIVER_UNLOAD primal_unload;

NO_DISCARD void find_arcane_pid();