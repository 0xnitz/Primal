#pragma once

#include "DefinesMacros.hpp"

namespace UnlinkLoadedModule
{
	void remove_from_loaded_modules(LIST_ENTRY PsLoadedModuleList, Address64 base_address);
};