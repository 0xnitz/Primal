#include "Primal.hpp"
#include "FunctionResolver.hpp"
#include "HandleProtection.hpp"

HANDLE ARCANE_PID = reinterpret_cast<HANDLE>(-1);
PSGETNEXTPROCESS PsGetNextProcess = nullptr;
PSGETPROCESSIMAGEFILENAME PsGetProcessImageFileName = nullptr;

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DEBUG_PRINT("In DriverEntry\n");

	UNICODE_STRING device_name = RTL_CONSTANT_STRING(L"\\Device\\Primal");
	UNICODE_STRING symbolic_link = RTL_CONSTANT_STRING(L"\\DosDevices\\Primal");
	PDEVICE_OBJECT device_object = nullptr;

	NTSTATUS status = IoCreateDevice(DriverObject,
		0,
		&device_name,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&device_object
	);

	if (!NT_SUCCESS(status)) 
	{
		DEBUG_PRINT("Failed to create device (0x%08X)\n", status);

		return status;
	}

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status)) 
	{
		DEBUG_PRINT("Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(device_object);

		return status;
	}

	status = FunctionResolver::resolve_ps_functions(DriverObject);
	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT("Failed to resolve critical functions!\n");

		return status;
	}

	find_arcane_pid();

	DriverObject->MajorFunction[IRP_MJ_CREATE] = primal_create_close;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = primal_create_close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = primal_control;
	DriverObject->DriverUnload = primal_unload;

	NTSTATUS register_result = HandleProtection::register_callback();
	if (!NT_SUCCESS(register_result))
	{
		DEBUG_PRINT("Critical fail registering Handle Protection!\n");

		return register_result;
	}

	DEBUG_PRINT("Driver loaded successfully!\n");

	return STATUS_SUCCESS;
}

NTSTATUS primal_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DEBUG_PRINT("IRP_MJ_CREATE/IRP_MJ_CLOSE\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS primal_control(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DEBUG_PRINT("IRP_MJ_DEVICE_CONTROL\n");

	UNUSED(PIO_STACK_LOCATION irpSp) = IoGetCurrentIrpStackLocation(Irp);

	Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_INVALID_DEVICE_REQUEST;
}

_Use_decl_annotations_ VOID primal_unload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	DEBUG_PRINT("DriverUnload\n");

	HandleProtection::unregister_callback();

	UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Primal");

	IoDeleteSymbolicLink(&symbolicLink);
	IoDeleteDevice(DriverObject->DeviceObject);
}

void find_arcane_pid()
{
	// TODO: somehow all pids are find besides ARCANE
	PEPROCESS process = PsInitialSystemProcess;

	do {
		const char* image_file_name = PsGetProcessImageFileName(process);

		if (image_file_name && _stricmp(image_file_name, ARCANE_PROCESS_NAME) == 0) {
			ARCANE_PID = PsGetProcessId(process);

			return;
		}

		process = PsGetNextProcess(process);

	} while (process != nullptr);

	DEBUG_PRINT("Arcane not found!\n");
}
