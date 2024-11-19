#include <ntddk.h>
#include <wdf.h>

extern "C" NTSTATUS device_add_callback(
	_In_    WDFDRIVER       Driver,
	_Inout_ PWDFDEVICE_INIT DeviceInit
)
{
	UNREFERENCED_PARAMETER(Driver);

	DbgPrintEx (DPFLTR_IHVDRIVER_ID, 3, "Primal: DeviceAdd Callback\n");

	WDFDEVICE h_device;
	const NTSTATUS status = WdfDeviceCreate(&DeviceInit,
		WDF_NO_OBJECT_ATTRIBUTES,
		&h_device
	);

	return status;
}

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	NTSTATUS status = STATUS_SUCCESS;
	WDF_DRIVER_CONFIG config;

	DbgPrintEx (DPFLTR_IHVDRIVER_ID, 3, "Primal: DriverEntry\n");

	status = WdfDriverCreate(DriverObject,
		RegistryPath,
		WDF_NO_OBJECT_ATTRIBUTES,
		&config,
		WDF_NO_HANDLE
	);

	DriverObject->DriverUnload = [](PDRIVER_OBJECT DriverObject) {
		UNREFERENCED_PARAMETER(DriverObject);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 3, "Primal: DriverUnload\n");
		};

	DriverObject->MajorFunction[IRP_MJ_CREATE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);
		UNREFERENCED_PARAMETER(Irp);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 3, "Primal: IRP_MJ_CREATE\n");

		return STATUS_SUCCESS;
		};

	DriverObject->MajorFunction[IRP_MJ_CLOSE] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);
		UNREFERENCED_PARAMETER(Irp);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 3, "Primal: IRP_MJ_CLOSE\n");

		return STATUS_SUCCESS;
		};

	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = [](PDEVICE_OBJECT DeviceObject, PIRP Irp) {
		UNREFERENCED_PARAMETER(DeviceObject);
		UNREFERENCED_PARAMETER(Irp);
		DbgPrintEx(DPFLTR_IHVDRIVER_ID, 3, "Primal: IRP_MJ_DEVICE_CONTROL\n");

		return STATUS_SUCCESS;
		};

	WDF_DRIVER_CONFIG_INIT(&config, device_add_callback);

	return status;
}