#pragma once

// Precompiled header

#include <cassert>

#ifdef __cplusplus
	#include <cstdbool>
	#include <cstdint>
#else
	#include <stdbool.h>
	#include <stdint.h>
#endif // __cplusplus


#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <mutex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>
#include <memory>
#include <optional>

#include "../VoxBox-Shared/Source/PlatformDetection.h"

// Macros are included here for internal global usage
// They're also defined under the API header file for external usage

#ifdef VB_PLATFORM_WINDOWS
	#ifdef VB_EXPORT_TTS
		#define VB_TTS_API VB_DLL_EXPORT
	#else
		#define VB_TTS_API VB_DLL_IMPORT
	#endif // VB_EXPORT_TTS
#endif // VB_PLATFORM_WINDOWS