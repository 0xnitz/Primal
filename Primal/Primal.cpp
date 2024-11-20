#include <ntddk.h>

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);

	DbgPrint("[Primal] In DriverEntry\n");

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

	if (!NT_SUCCESS(status)) {
		DbgPrint("[Primal] Failed to create device (0x%08X)\n", status);

		return status;
	}

	status = IoCreateSymbolicLink(&symbolic_link, &device_name);
	if (!NT_SUCCESS(status)) {
		DbgPrint("[Primal] Failed to create symbolic link (0x%08X)\n", status);
		IoDeleteDevice(device_object);

		return status;
	}

	DriverObject->DriverUnload = [](PDRIVER_OBJECT DriverObject) {
		UNREFERENCED_PARAMETER(DriverObject);

		DbgPrint("[Primal] DriverUnload\n");

		UNICODE_STRING symbolicLink = RTL_CONSTANT_STRING(L"\\DosDevices\\Primal");

		IoDeleteSymbolicLink(&symbolicLink);
		IoDeleteDevice(DriverObject->DeviceObject);
		
		};

	DriverObject->MajorFunction[IRP_MJ_CREATE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);

		DbgPrint("[Primal] IRP_MJ_CREATE\n");

		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
		};

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);

		DbgPrint("[Primal] IRP_MJ_CLOSE\n");

		Irp->IoStatus.Status = STATUS_SUCCESS;
		Irp->IoStatus.Information = 0;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_SUCCESS;
		};

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);

		DbgPrint("[Primal] IRP_MJ_DEVICE_CONTROL\n");

		[[maybe_unused]] PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation(Irp);

		Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
		Irp->IoStatus.Information = 0;

		IoCompleteRequest(Irp, IO_NO_INCREMENT);

		return STATUS_INVALID_DEVICE_REQUEST;
		};

	DbgPrint("[Primal] Driver loaded successfully!\n");

	return STATUS_SUCCESS;
}