#include "Memory.hpp"

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
	DEBUG_PRINT("(0x%08X) to (0x%08X)", physical_address, user_buffer);

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

}