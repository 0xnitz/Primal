#include "UnlinkLoadedModule.hpp"

namespace UnlinkLoadedModule
{
void remove_from_loaded_modules(LIST_ENTRY loaded_modules, Address64 base_address)
{
	// TODO: somehow this causes an infinite loop if the module is not found, debug this.
	for (PLIST_ENTRY current = loaded_modules.Flink;
		current != &loaded_modules;
		current = current->Flink)
	{
		PKLDR_DATA_TABLE_ENTRY module =
			CONTAINING_RECORD(current, KLDR_DATA_TABLE_ENTRY, InLoadOrderLinks);

		if (module->DllBase == reinterpret_cast<PVOID>(base_address)) {
			RemoveEntryList(current);
			current->Flink = current;
			current->Blink = current;

			DEBUG_PRINT("Detached Primal from PsLoadedModuleList!");

			return;
		}
	}
}
}