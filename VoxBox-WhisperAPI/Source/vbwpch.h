#pragma once

// Precompiled header

#include <cassert>
#include <cctype>
#include <cstring>
#include <string>
#include <vector>


#include "../VoxBox-Shared/Source/PlatformDetection.h"

// Macros are included here for internal global usage
// They're also defined under the API header file for external usage
#ifdef VB_EXPORT_STT
	#define VB_STT_API VB_DLL_EXPORT
#else
	#define VB_STT_API VB_DLL_IMPORT
#endif // VB_EXPORT_STT