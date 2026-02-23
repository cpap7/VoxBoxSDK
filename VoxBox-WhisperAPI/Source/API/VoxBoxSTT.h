#pragma once

// Included under API header file for external usage
#ifdef _WIN32
	#ifdef VB_EXPORT_STT
		#define VB_STT_API __declspec(dllexport)
	#else
		#define VB_STT_API __declspec(dllimport)
	#endif
#else
	#define VB_EXPORT_STT
	#ifndef __stdcall
		#define __stdcall
	#endif
#endif

// For C & C++ include headers
#ifdef __cplusplus
	#include <cstddef>
	#include <cstdint>
	#include <cstdbool>
#else
	#include <stddef.h>
	#include <stdint.h>
	#include <stdbool.h>
#endif



#ifdef __cplusplus
extern "C" {
#endif





#ifdef __cplusplus
}
#endif
