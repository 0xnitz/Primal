#include "HandleProtection.hpp"

POB_CALLBACK_REGISTRATION CALLBACK_REGISTRATION = nullptr;

static constexpr UINT32 HOOK_OPERATION_LENGTH = 1;
OB_OPERATION_REGISTRATION OPERATION_REGISTRATION[HOOK_OPERATION_LENGTH];

_Use_decl_annotations_ OB_PREOP_CALLBACK_STATUS OpenProcessHook(
	PVOID RegistrationContext,
	POB_PRE_OPERATION_INFORMATION PreInfo
) {
	UNREFERENCED_PARAMETER(RegistrationContext);

	if (PreInfo->ObjectType == *PsProcessType)
	{
		auto target_process = static_cast<PEPROCESS>(PreInfo->Object);

		const char* target_process_name = PsGetProcessImageFileName(target_process);
		if (target_process_name && _stricmp(target_process_name, ARCANE_PROCESS_NAME) == 0)
		{
			if (PreInfo->Operation == OB_OPERATION_HANDLE_CREATE ||
				PreInfo->Operation == OB_OPERATION_HANDLE_DUPLICATE)
			{
				DEBUG_PRINT("Handle Block from %d\n", PsGetCurrentProcessId());

				PreInfo->Parameters->CreateHandleInformation.DesiredAccess = 0;
				PreInfo->Parameters->CreateHandleInformation.OriginalDesiredAccess = 0;
			}
		}
	}

	return OB_PREOP_SUCCESS;
}

NTSTATUS HandleProtection::register_callback()
{
	RtlZeroMemory(OPERATION_REGISTRATION, sizeof(OPERATION_REGISTRATION));
	OPERATION_REGISTRATION[0].ObjectType = PsProcessType;
	OPERATION_REGISTRATION[0].Operations = OB_OPERATION_HANDLE_CREATE | OB_OPERATION_HANDLE_DUPLICATE;
	OPERATION_REGISTRATION[0].PreOperation = OpenProcessHook;
	OPERATION_REGISTRATION[0].PostOperation = nullptr;

	OB_CALLBACK_REGISTRATION callback = {};
	callback.Version = OB_FLT_REGISTRATION_VERSION;
	callback.OperationRegistrationCount = 1;
	callback.OperationRegistration = OPERATION_REGISTRATION;
	callback.RegistrationContext = nullptr;

	NTSTATUS out_status = ObRegisterCallbacks(&callback, reinterpret_cast<PVOID*>(&CALLBACK_REGISTRATION));
	if (!NT_SUCCESS(out_status)) {
		DEBUG_PRINT("Failed to register Ob callback: 0x%X!\n", out_status);

		return out_status;
	}

	DEBUG_PRINT("Ob callback registered\n");

	return out_status;
}

void HandleProtection::unregister_callback()
{
	if (CALLBACK_REGISTRATION) {
		ObUnRegisterCallbacks(CALLBACK_REGISTRATION);

		CALLBACK_REGISTRATION = nullptr;
	}

	DEBUG_PRINT("Ob callback unregistered\n");
}
