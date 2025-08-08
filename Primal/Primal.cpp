#include "Primal.hpp"
#include "DetachEprocess.hpp"
#include "FunctionResolver.hpp"
#include "HandleProtection.hpp"
#include "UnlinkLoadedModule.hpp"

HANDLE ARCANE_PID = reinterpret_cast<HANDLE>(-1);
PEPROCESS ARCANE_PROCESS = nullptr;
PSGETNEXTPROCESS PsGetNextProcess = nullptr;
PSGETPROCESSIMAGEFILENAME PsGetProcessImageFileName = nullptr;
KSPIN_LOCK PRIMAL_LOCK;
UNICODE_STRING DEVICE_NAME;
UNICODE_STRING SYMBOLIC_LINK;

extern "C" NTSTATUS DriverEntry(
	_In_ PDRIVER_OBJECT     DriverObject,
	_In_ PUNICODE_STRING    RegistryPath
)
{
	UNREFERENCED_PARAMETER(RegistryPath);
	
	DEBUG_PRINT_OBFUSCATE("In DriverEntry\n");

	auto device_name_obfuscated = WOBFUSCATE("\\Device\\Primal");
	RtlInitUnicodeString(&DEVICE_NAME, device_name_obfuscated);
	auto symbolic_link_obfuscated = WOBFUSCATE("\\DosDevices\\Primal");
	RtlInitUnicodeString(&SYMBOLIC_LINK, symbolic_link_obfuscated);
	PDEVICE_OBJECT device_object = nullptr;

	NTSTATUS status = RESOLVE(IoCreateDevice)(DriverObject,
		0,
		&DEVICE_NAME,
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&device_object
	);

	if (!NT_SUCCESS(status)) 
	{
		DEBUG_PRINT_OBFUSCATE("Failed to create device ");
		DEBUG_PRINT("(0x%08X)\n", status);

		return status;
	}

	status = RESOLVE(IoCreateSymbolicLink)(&SYMBOLIC_LINK, &DEVICE_NAME);
	if (!NT_SUCCESS(status)) 
	{
		DEBUG_PRINT_OBFUSCATE("Failed to create symbolic link ");
		DEBUG_PRINT("(0x%08X)\n", status);
		RESOLVE(IoDeleteDevice)(device_object);

		return status;
	}

	KeInitializeSpinLock(&PRIMAL_LOCK);

	status = FunctionResolver::resolve_ps_functions(DriverObject);
	if (!NT_SUCCESS(status))
	{
		DEBUG_PRINT_OBFUSCATE("Failed to resolve critical functions!\n");

		return status;
	}

	find_arcane_pid();

	DriverObject->MajorFunction[IRP_MJ_CREATE] = primal_create_close;
	DriverObject->MajorFunction[IRP_MJ_CLOSE] = primal_create_close;
	DriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = primal_control;
	DriverObject->MajorFunction[IRP_MJ_READ] = Memory::primal_read;
	DriverObject->MajorFunction[IRP_MJ_WRITE] = Memory::primal_write;
	DriverObject->DriverUnload = primal_unload;

	NTSTATUS register_result = HandleProtection::register_callback();
	if (!NT_SUCCESS(register_result))
	{
		DEBUG_PRINT_OBFUSCATE("Critical fail registering Handle Protection!\n");

		return register_result;
	}

	DetachEprocess::remove_from_process_links(ARCANE_PROCESS);
	UnlinkLoadedModule::remove_from_loaded_modules(PsLoadedModuleList,
		reinterpret_cast<Address64>(DriverObject->DriverStart));

	DEBUG_PRINT_OBFUSCATE("Driver loaded successfully!\n");

	return STATUS_SUCCESS;
}

NTSTATUS primal_create_close(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DEBUG_PRINT_OBFUSCATE("IRP_MJ_CREATE/IRP_MJ_CLOSE\n");

	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_SUCCESS;
}

NTSTATUS primal_control(PDEVICE_OBJECT DeviceObject, PIRP Irp)
{
	UNREFERENCED_PARAMETER(DeviceObject);

	DEBUG_PRINT_OBFUSCATE("IRP_MJ_DEVICE_CONTROL\n");

	UNUSED(PIO_STACK_LOCATION irpSp) = IoGetCurrentIrpStackLocation(Irp);

	Irp->IoStatus.Status = STATUS_INVALID_DEVICE_REQUEST;
	Irp->IoStatus.Information = 0;

	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	return STATUS_INVALID_DEVICE_REQUEST;
}

_Use_decl_annotations_ VOID primal_unload(PDRIVER_OBJECT DriverObject)
{
	UNREFERENCED_PARAMETER(DriverObject);

	DEBUG_PRINT_OBFUSCATE("DriverUnload\n");

	HandleProtection::unregister_callback();

	RESOLVE(IoDeleteSymbolicLink)(&SYMBOLIC_LINK);
	RESOLVE(IoDeleteDevice)(DriverObject->DeviceObject);
}

void find_arcane_pid()
{
	PEPROCESS process = PsInitialSystemProcess;
	const char* arcane_name = ARCANE_PROCESS_NAME;

	do {
		const char* image_file_name = PsGetProcessImageFileName(process);

		if (image_file_name && _stricmp(image_file_name, arcane_name) == 0) {
			ARCANE_PID = PsGetProcessId(process);
			ARCANE_PROCESS = process;

			return;
		}

		process = PsGetNextProcess(process);

	} while (process != nullptr);

	DEBUG_PRINT_OBFUSCATE("Arcane not found!\n");
}
