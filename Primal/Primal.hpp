#pragma once

// TODO: add nice error codes

#include "Memory.hpp"
#include "DefinesMacros.hpp"

NO_DISCARD NTSTATUS primal_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NO_DISCARD NTSTATUS primal_control(PDEVICE_OBJECT DeviceObject, PIRP Irp);

NO_DISCARD DRIVER_UNLOAD primal_unload;

void find_arcane_pid();