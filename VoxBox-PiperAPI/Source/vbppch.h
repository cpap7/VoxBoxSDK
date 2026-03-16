#pragma once

// Precompiled header -- used internally for .cpp files

//  Platform Detection 
#ifdef _WIN32
	#define VB_PLATFORM_WINDOWS
	#define WIN32_LEAN_AND_MEAN
	#define NOMINMAX
#elif defined(__APPLE__)
	#define VB_PLATFORM_MACOS
#elif defined(__linux__)
	#define VB_PLATFORM_LINUX
#endif

// Platform-Specific Headers
#ifdef VB_PLATFORM_WINDOWS
	#include <windows.h>

#elif defined(VB_PLATFORM_MACOS)
	#include <mach-o/dyld.h>
	#include <climits>  // For PATH_MAX
#elif defined(VB_PLATFORM_LINUX)
	#include <climits>  // For PATH_MAX
#endif

// Standard Library 
#ifdef __cplusplus
	#include <cstdint>
	#include <cstdbool>
#else
	#include <stdint.h>
	#include <stdbool.h>
#endif

#include <cassert>
#include <condition_variable>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include <VBCommon.h>

// DLL Macros
#ifdef VB_PLATFORM_WINDOWS
	#define VB_DLL_EXPORT __declspec(dllexport)
	#define VB_DLL_IMPORT __declspec(dllimport)
	#define VB_CALL __stdcall
#else
	#define VB_DLL_EXPORT
	#define VB_DLL_IMPORT
	#define VB_CALL
#endif

#ifdef VB_EXPORT_TTS
	#define VB_TTS_API VB_DLL_EXPORT
#else
	#define VB_TTS_API VB_DLL_IMPORT
#endif