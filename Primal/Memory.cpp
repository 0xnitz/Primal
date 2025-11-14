#include "Memory.hpp"
#include "KernelUtils.hpp"

namespace Memory
{

NO_DISCARD NTSTATUS primal_read(UNUSED(PDEVICE_OBJECT DeviceObject), PIRP Irp)
{
	// TODO: read via pte remapping
	PIO_STACK_LOCATION irp_stack_location = IoGetCurrentIrpStackLocation(Irp);
	ULONG_PTR size = irp_stack_location->Parameters.Read.Length;
	PHYSICAL_ADDRESS physical_address = irp_stack_location->Parameters.Read.ByteOffset;

	if (size == 0 || size > PAGE_SIZE) {
		DEBUG_PRINT_OBFUSCATE("Bad read params!\n");

		COMPLETE_REQUEST(Irp, STATUS_INVALID_PARAMETER)
	}

	LOCK();

	// TODO: resolve doesn't work on mapiospace api although they are exported from ntos
	PVOID mapped_page = MmMapIoSpace(physical_address, size, MmNonCached);
	if (!mapped_page) {
		DEBUG_PRINT_OBFUSCATE("Error MmMapIoSpace!\n");

		COMPLETE_REQUEST_UNLOCK(Irp, STATUS_INSUFFICIENT_RESOURCES)
	}

	PVOID user_buffer = Irp->UserBuffer;
	if (!user_buffer) {
		DEBUG_PRINT_OBFUSCATE("Bad UM read params!\n");

		MmUnmapIoSpace(mapped_page, size);

		COMPLETE_REQUEST_UNLOCK(Irp, STATUS_INVALID_USER_BUFFER)
	}

	RtlCopyMemory(user_buffer, mapped_page, size);
	Irp->IoStatus.Information = size;
	MmUnmapIoSpace(mapped_page, size);

	DEBUG_PRINT_OBFUSCATE("Mapping address ");
	DEBUG_PRINT("(0x%08X) to (0x%08X)\n", physical_address, user_buffer);

	COMPLETE_REQUEST_UNLOCK(Irp, STATUS_SUCCESS)
}

NO_DISCARD NTSTATUS primal_write(UNUSED(PDEVICE_OBJECT DeviceObject), PIRP Irp)
{
	PIO_STACK_LOCATION irp_stack_location = IoGetCurrentIrpStackLocation(Irp);
	ULONG_PTR size = irp_stack_location->Parameters.Read.Length;
	PHYSICAL_ADDRESS physical_address = irp_stack_location->Parameters.Read.ByteOffset;

	if (size == 0 || size > PAGE_SIZE) {
		DEBUG_PRINT_OBFUSCATE("Bad write params!\n");

		COMPLETE_REQUEST(Irp, STATUS_INVALID_PARAMETER)
	}

	LOCK();

	PVOID mapped_page = MmMapIoSpace(physical_address, size, MmNonCached);
	if (!mapped_page) {
		DEBUG_PRINT_OBFUSCATE("Error MmMapIoSpace!\n");

		COMPLETE_REQUEST_UNLOCK(Irp, STATUS_INSUFFICIENT_RESOURCES)
	}

	PVOID user_buffer = Irp->UserBuffer;
	if (!user_buffer) {
		DEBUG_PRINT_OBFUSCATE("Bad UM write buffer!\n");

		MmUnmapIoSpace(mapped_page, size);

		COMPLETE_REQUEST_UNLOCK(Irp, STATUS_INVALID_USER_BUFFER)
	}

	RtlCopyMemory(mapped_page, user_buffer, size);
	Irp->IoStatus.Information = size;
	MmUnmapIoSpace(mapped_page, size);

	DEBUG_PRINT_OBFUSCATE("Writing user data to ");
	DEBUG_PRINT("(0x%08X)", physical_address);

	COMPLETE_REQUEST_UNLOCK(Irp, STATUS_SUCCESS)
}

NO_DISCARD NTSTATUS primal_read_virtual(HANDLE pid, Address64 address, void* buffer, size_t size)  
{
	KAPC_STATE state;
	SIZE_T bytes_copied = 0;
	MM_COPY_ADDRESS source_address;
	source_address.VirtualAddress = reinterpret_cast<PVOID>(address);

	PRKPROCESS process = reinterpret_cast<PRKPROCESS>(KernelUtils::get_process_of_pid(pid));
	KeStackAttachProcess(process, &state);

	NTSTATUS status = MmCopyMemory(buffer, source_address, size, MM_COPY_MEMORY_VIRTUAL, &bytes_copied);
	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT_OBFUSCATE("Failed to read virtual memory!\n");
		KeUnstackDetachProcess(&state);

		return status;
	}

	KeUnstackDetachProcess(&state);
	DEBUG_PRINT_OBFUSCATE("Read virtual memory successfully!\n");

	return STATUS_SUCCESS;
}

NO_DISCARD NTSTATUS primal_write_virtual(HANDLE pid, Address64 address, void* buffer, size_t size)
{
	KAPC_STATE state;
	PRKPROCESS process = reinterpret_cast<PRKPROCESS>(KernelUtils::get_process_of_pid(pid));
	KeStackAttachProcess(process, &state);
	size_t copied = 0;

	NTSTATUS copy_status = MmCopyVirtualMemory(
		PsGetCurrentProcess(),
		reinterpret_cast<PVOID>(buffer),
		reinterpret_cast<PEPROCESS>(process),
		reinterpret_cast<PVOID>(address),
		size,
		KernelMode,
		&copied);

	if (!NT_SUCCESS(copy_status) || copied != size)
	{
		DEBUG_PRINT_OBFUSCATE("Failed to write virtual memory!\n");
		KeUnstackDetachProcess(&state);

		return STATUS_ACCESS_VIOLATION;
	}

	KeUnstackDetachProcess(&state);

	return STATUS_SUCCESS;
}

NO_DISCARD Address64 primal_allocate_virtual(HANDLE pid, size_t size, ULONG allocation_type, ULONG protect)
{
	KAPC_STATE state;
	Address64 allocated_memory = 0;
	PRKPROCESS process = reinterpret_cast<PRKPROCESS>(KernelUtils::get_process_of_pid(pid));
	KeStackAttachProcess(process, &state);
	
	NTSTATUS status = ZwAllocateVirtualMemory(NtCurrentProcess(), reinterpret_cast<PVOID*>(&allocated_memory), 0, &size, allocation_type, protect);
	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT_OBFUSCATE("Failed to allocate virtual memory!\n");
		KeUnstackDetachProcess(&state);

		return 0;
	}

	KeUnstackDetachProcess(&state);

	return allocated_memory;
}

}