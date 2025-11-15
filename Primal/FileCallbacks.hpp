#pragma once

#include "DefinesMacros.hpp"

extern KESTACKATTACHPROCESS KeStackAttachProcess;
extern KEUNSTACKDETACHPROCESS KeUnstackDetachProcess;
extern PSGETPROCESSIMAGEFILENAME PsGetProcessImageFileName;

namespace FileCallbacks
{
	void load_image_notify_routine(
		PUNICODE_STRING FullImageName,
		HANDLE ProcessId,
		PIMAGE_INFO ImageInfo
	);

	void set_load_image_notify_routine();

	void remove_load_image_notify_routine();

	NTSTATUS hook_iat_of_module(HANDLE pid, Address64 module_base, const char* function_name, void* hook_function, void** original_function, void** address_in_table);

	int read_file_hook(HANDLE hFile, Address64 lpBuffer, DWORD32 nNumberOfBytesToRead, DWORD32* lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped);

	Address64 map_view_of_file_hook(HANDLE hFileMappingObject, DWORD32 dwDesiredAccess, DWORD32 dwFileOffsetHigh, DWORD32 dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap);

	bool allocate_fake_file_mapping(HANDLE pid);
};