#pragma once

#ifdef _WIN32 // Windows x86
    #define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers.
    
    #ifdef _MSC_VER
        #define NOMINMAX
        #include <Windows.h>
    #endif // _MSC_VER
    
    #include <fcntl.h>
    #include <io.h>
    
    #define VB_PLATFORM_WINDOWS
    #define VB_STDCALL __stdcall
    
// Have to check __ANDROID__ before __linux__ 
// since it's based on the Linux kernel 
// and has it defined
#elif defined(__ANDROID__)
    #define VB_PLATFORM_ANDROID
    #define VB_STDCALL

#elif defined(__linux__)
    #define VB_PLATFORM_LINUX   
    #define VB_STDCALL 

#elif defined(__APPLE__) || defined(__MACH__)
    #include <mach-o/dyld.h>
    #include <TargetConditionals.h>
    /* TARGET_OS_MAC exists on all platforms,
    * so all different Apple platforms must be
    * checked to ensure it's running on MACOS, not
    * IOS or some other Apple platform
    */
    #if TARGET_IPHONE_SIMULATOR == 1
        #define VB_PLATFORM_IOS_SIMULATOR
        #define VB_STDCALL 

    #elif TARGET_OS_IPHONE == 1
        #define VB_PLATFORM_IOS
        #define VB_STDCALL 

    #elif TARGET_OS_MAC == 1
        #define VB_PLATFORM_MACOS
        #define VB_STDCALL

    #else
        #error "Unknown Apple platform!"
    #endif 
#endif // _WIN32

// DLL macros
#ifdef VB_PLATFORM_WINDOWS
    #define VB_DLL_EXPORT __declspec(dllexport)
    #define VB_DLL_IMPORT __declspec(dllimport)
#else
    #define VB_DLL_EXPORT __attribute__((visibility("default")))
    #define VB_DLL_IMPORT
#endif // VB_PLATFORM_WINDOWS


