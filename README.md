* prettify and general refactor
* change function resolving process
* optimize stirng obfuscation and obfuscate all
* protections for KM
	* UEFI runtime driver launched from UEFIShell? (something close to efiguard)
	* protect from MmCopyVirtualMemory and it's buddies
	* load reflective driver
	* flip the order, uefi runtime -> primal -> arcane
* queue apc for usermode that injects arcane (flip the load order, make another small usermode stage that loads the driver) -> driver deploys full arcane
* actually use the protections/hooking services I've made to hide Arcane.exe
* make better cleanup for UM memory artifcats (hooks, allocations, IAT protection, etc..)