#pragma once

#include "DefinesMacros.hpp"

extern PSGETPROCESSIMAGEFILENAME PsGetProcessImageFileName;

namespace HandleProtection
{
	NO_DISCARD NTSTATUS register_callback();

	void unregister_callback();
};