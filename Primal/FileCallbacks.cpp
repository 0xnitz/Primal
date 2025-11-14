#include "Memory.hpp"
#include "KernelUtils.hpp"
#include "FileCallbacks.hpp"

#include <ntimage.h>

namespace FileCallbacks
{

Address64 ORIGINAL_FUNCTION_ADDRESS = 0;
Address64 ORIGINAL_FUNCTION = 0;
HANDLE NOTEPAD_PID = 0;
Address64 FAKE_FILE_MAPPING = 0;

void load_image_notify_routine(PUNICODE_STRING FullImageName, HANDLE ProcessId, PIMAGE_INFO ImageInfo)
{
	PEPROCESS process = KernelUtils::get_process_of_pid(ProcessId);
	const char* image_file_name = PsGetProcessImageFileName(process);

	if (_stricmp(image_file_name, OBFUSCATE("Notepad.exe")) == 0 && FullImageName && ImageInfo && ImageInfo->ImageBase)
	{
		if (wcsstr(FullImageName->Buffer, L"ntdll.dll") || wcsstr(FullImageName->Buffer, WOBFUSCATE("NTDLL.DLL")))
		{
			DEBUG_PRINT_OBFUSCATE("Notepad.exe ntdll loaded, hooking IAT of image\n");
			DEBUG_PRINT_OBFUSCATE("Allocating fake file mapping\n");
			allocate_fake_file_mapping(ProcessId);

			Address64 shellcode_address = Memory::primal_allocate_virtual(ProcessId, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
			if (!shellcode_address)
			{
				DEBUG_PRINT_OBFUSCATE("Failed to allocate memory in usermode for shellcode!\n");

				return;
			}

			DEBUG_PRINT_OBFUSCATE("allocated memory\n");
			NTSTATUS write_status = Memory::primal_write_virtual(
				ProcessId,
				shellcode_address,
				(void*)map_view_of_file_hook,
				PAGE_SIZE);
			if (write_status != STATUS_SUCCESS)
			{
				DEBUG_PRINT_OBFUSCATE("Failed to write shellcode to usermode!\n");

				return;
			}

			DEBUG_PRINT_OBFUSCATE("write memory success\n");
			NOTEPAD_PID = ProcessId;
			PPEB peb = KernelUtils::get_peb_of_process(process);
			if (!peb)
			{
				DEBUG_PRINT_OBFUSCATE("Failed to get PEB of Notepad.exe!\n");

				return;
			}
			Address64 notepad_base_address = *reinterpret_cast<Address64*>((reinterpret_cast<Address64>(peb) + 0x10));
			
			DEBUG_PRINT_OBFUSCATE("peb success\n");
			NTSTATUS hook_iat = hook_iat_of_module(
				ProcessId,
				notepad_base_address,
				OBFUSCATE("MapViewOfFile"),
				reinterpret_cast<void*>(shellcode_address),
				reinterpret_cast<void**>(&ORIGINAL_FUNCTION_ADDRESS),
				reinterpret_cast<void**>(&ORIGINAL_FUNCTION));
			if (hook_iat != STATUS_SUCCESS)
			{
				DEBUG_PRINT_OBFUSCATE("Failed to hook IAT of Notepad.exe!\n");

				return;
			}
		}
	}
}

void set_load_image_notify_routine()
{
	PsSetLoadImageNotifyRoutine(load_image_notify_routine);

	DEBUG_PRINT_OBFUSCATE("Load Image Callback in place!\n");
}

void remove_load_image_notify_routine()
{
	PsRemoveLoadImageNotifyRoutine(load_image_notify_routine);

	NTSTATUS status = Memory::primal_write_virtual(NOTEPAD_PID, ORIGINAL_FUNCTION, &ORIGINAL_FUNCTION_ADDRESS, sizeof(Address64));
	if (status != STATUS_SUCCESS)
	{
		DEBUG_PRINT_OBFUSCATE("Failed to unhook IAT!\n");

		return;
	}

	DEBUG_PRINT_OBFUSCATE("Unhooked IAT successfully!\n");
}

NTSTATUS hook_iat_of_module(HANDLE pid, Address64 module_base, const char* function_name, void* hook_function, void** original_function, void** address_in_table)
{
	KAPC_STATE state;
	PRKPROCESS process = reinterpret_cast<PRKPROCESS>(KernelUtils::get_process_of_pid(pid));
	KeStackAttachProcess(process, &state);

	PIMAGE_DOS_HEADER dos_header = reinterpret_cast<PIMAGE_DOS_HEADER>(module_base);
	if (dos_header->e_magic != IMAGE_DOS_SIGNATURE)
	{
		KeUnstackDetachProcess(&state);

		return STATUS_INVALID_IMAGE_FORMAT;
	}

	PIMAGE_NT_HEADERS64 nt_headers = reinterpret_cast<PIMAGE_NT_HEADERS64>(module_base + dos_header->e_lfanew);
	if (nt_headers->Signature != IMAGE_NT_SIGNATURE)
	{
		KeUnstackDetachProcess(&state);

		return STATUS_INVALID_IMAGE_FORMAT;
	}

	IMAGE_DATA_DIRECTORY import_directory = nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT];
	if (import_directory.VirtualAddress == 0 || import_directory.Size == 0)
	{
		KeUnstackDetachProcess(&state);

		return STATUS_INVALID_IMAGE_FORMAT;
	}

	PIMAGE_IMPORT_DESCRIPTOR import_desc = reinterpret_cast<PIMAGE_IMPORT_DESCRIPTOR>(module_base + import_directory.VirtualAddress);
	if (!import_desc)
	{
		KeUnstackDetachProcess(&state);

		return STATUS_INVALID_IMAGE_FORMAT;
	}

	for (; import_desc->Name != 0; ++import_desc)
	{
		ULONG origFirstThunkRva = import_desc->OriginalFirstThunk;
		ULONG firstThunkRva = import_desc->FirstThunk;

		PIMAGE_THUNK_DATA64 iat_thunk = nullptr;
		if (origFirstThunkRva != 0)
		{
			iat_thunk = reinterpret_cast<PIMAGE_THUNK_DATA64>(module_base + origFirstThunkRva);
		} else
		{
			iat_thunk = reinterpret_cast<PIMAGE_THUNK_DATA64>(module_base + firstThunkRva);
		}	

		for (; iat_thunk->u1.AddressOfData != 0; ++iat_thunk)
		{
			if (iat_thunk->u1.Ordinal & IMAGE_ORDINAL_FLAG64)
			{
				continue;
			}

			PIMAGE_IMPORT_BY_NAME ibn = reinterpret_cast<PIMAGE_IMPORT_BY_NAME>(module_base + iat_thunk->u1.AddressOfData);
			if (!ibn || ibn->Name == nullptr)
			{
				continue;
			}

			const char* imported_name = reinterpret_cast<const char*>(ibn->Name);
			if (_stricmp(imported_name, function_name) == 0)
			{
				Address64 iat_entry_va = reinterpret_cast<Address64>(iat_thunk);
				Address64 original = *reinterpret_cast<Address64*>(iat_entry_va);
				DEBUG_PRINT_OBFUSCATE("Found IAT entry, hooking...\n");

				NTSTATUS write_status = Memory::primal_write_virtual(pid, iat_entry_va, &hook_function, sizeof(Address64));
				if (write_status != STATUS_SUCCESS)
				{
					DEBUG_PRINT_OBFUSCATE("Failed to hook IAT entry!\n");
					KeUnstackDetachProcess(&state);

					return write_status;
				}
				
				*original_function = reinterpret_cast<void*>(original);
				*address_in_table = reinterpret_cast<void*>(iat_entry_va);

				KeUnstackDetachProcess(&state);

				return STATUS_SUCCESS;
			}
		}
	}

	KeUnstackDetachProcess(&state);

	return STATUS_NOT_FOUND;
}

bool allocate_fake_file_mapping(HANDLE pid)
{
	Address64 fake_file_mapping = Memory::primal_allocate_virtual(pid, PAGE_SIZE, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	if (!fake_file_mapping)
	{
		DEBUG_PRINT_OBFUSCATE("Failed to allocate fake file mapping!\n");

		return false;
	}

	Byte fake_mapping_data[PAGE_SIZE] = { 0 };
	NTSTATUS write_status = Memory::primal_write_virtual(pid, fake_file_mapping, reinterpret_cast<void*>(fake_mapping_data), PAGE_SIZE);
	if (write_status != STATUS_SUCCESS)
	{
		DEBUG_PRINT_OBFUSCATE("Failed to write fake file mapping data!\n");

		return false;
	}

	FAKE_FILE_MAPPING = fake_file_mapping;

	return true;
}

Address64 map_view_of_file_hook(UNUSED(HANDLE hFileMappingObject), UNUSED(DWORD32 dwDesiredAccess), UNUSED(DWORD32 dwFileOffsetHigh), UNUSED(DWORD32 dwFileOffsetLow), SIZE_T dwNumberOfBytesToMap)
{
	if (dwNumberOfBytesToMap > PAGE_SIZE)
	{
		return NULL;
	}

	return FAKE_FILE_MAPPING;
}

int read_file_hook(UNUSED(HANDLE hFile), UNUSED(Address64 lpBuffer), UNUSED(DWORD32 nNumberOfBytesToRead), UNUSED(DWORD32* lpNumberOfBytesRead), UNUSED(LPOVERLAPPED lpOverlapped))
{
	// Check if the file name is the target file, if so, return zero, else call the original function.

	if (nNumberOfBytesToRead == 0x1337)
	{
		*reinterpret_cast<char**>(lpBuffer)[0] = 'N';
	}

	return STATUS_SUCCESS;
}

}