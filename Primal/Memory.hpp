#pragma once

#include "DefinesMacros.hpp"

extern KESTACKATTACHPROCESS KeStackAttachProcess;
extern KEUNSTACKDETACHPROCESS KeUnstackDetachProcess;
extern ZWALLOCATEVIRTUALMEMORY ZwAllocateVirtualMemory;

namespace Memory
{
	NO_DISCARD NTSTATUS primal_read(PDEVICE_OBJECT DeviceObject, PIRP Irp);

	NO_DISCARD NTSTATUS primal_write(PDEVICE_OBJECT DeviceObject, PIRP Irp);

	NO_DISCARD NTSTATUS primal_read_virtual(HANDLE pid, Address64 address, void* buffer, size_t size);

	NO_DISCARD NTSTATUS primal_write_virtual(HANDLE pid, Address64 address, void* buffer, size_t size);

	NO_DISCARD Address64 primal_allocate_virtual(HANDLE pid, size_t size, ULONG allocation_type, ULONG protect);
};