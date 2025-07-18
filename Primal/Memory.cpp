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
		DEBUG_PRINT("Bad read params!\n");

		COMPLETE_REQUEST(Irp, STATUS_INVALID_PARAMETER)
	}

	PVOID mapped_page = MmMapIoSpace(physical_address, size, MmNonCached);
	if (!mapped_page) {
		DEBUG_PRINT("Error MmMapIoSpace!\n");

		COMPLETE_REQUEST(Irp, STATUS_INSUFFICIENT_RESOURCES)
	}

	PVOID user_buffer = Irp->UserBuffer;
	if (!user_buffer) {
		DEBUG_PRINT("Bad UM read params!\n");

		MmUnmapIoSpace(mapped_page, size);

		COMPLETE_REQUEST(Irp, STATUS_INVALID_USER_BUFFER)
	}

	RtlCopyMemory(user_buffer, mapped_page, size);
	Irp->IoStatus.Information = size;
	MmUnmapIoSpace(mapped_page, size);

	DEBUG_PRINT("Mapping address (0x%08X) to UM (0x%08X)", physical_address, user_buffer);

	COMPLETE_REQUEST(Irp, STATUS_SUCCESS)
}

}