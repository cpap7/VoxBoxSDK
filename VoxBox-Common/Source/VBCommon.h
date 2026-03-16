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
    // Speech-to-text API
    #ifdef VB_EXPORT_STT
        #define VB_STT_API __declspec(dllexport)
    #else
        #define VB_STT_API __declspec(dllimport)
    #endif

    // Text-to-speech API
    #ifdef VB_EXPORT_TTS
        #define VB_TTS_API __declspec(dllexport)
    #else
        #define VB_TTS_API __declspec(dllimport)
    #endif

    // LLM API
    #ifdef VB_EXPORT_LLM
        #define VB_LLM_API __declspec(dllexport)
    #else
        #define VB_LLM_API __declspec(dllimport)
    #endif

    // Common macros
    #define VB_CALL __stdcall
#else
    #define VB_STT_API
    #define VB_TTS_API
    #define VB_LLM_API
    #define VB_CALL
#endif // VB_PLATFORM_WINDOWS


#ifdef __cplusplus
#include <cstdint>
#include <functional>

namespace VoxBox {
    // Common result codes
    enum class EResultCode : uint8_t {
        Unknown = 0,
        
        Success,
        NotInitialized,
        InvalidParameter,
        ModelLoadFailed,
        ProcessingFailed,
        Cancelled,
        OutOfMemory,
        InternalError
        
    };

    inline std::string ResultCodeToString(EResultCode a_result_code) {        
        switch (a_result_code) {
            case EResultCode::Success:          return "SUCCESS";
            case EResultCode::NotInitialized:   return "NOT INITIALIZED";
            case EResultCode::InvalidParameter: return "INVALID PARAMETER(S)";
            case EResultCode::ModelLoadFailed:  return "MODEL LOAD FAILED";
            case EResultCode::ProcessingFailed: return "PROCESSING FAILED";
            case EResultCode::Cancelled:        return "CANCELLED";
            case EResultCode::OutOfMemory:      return "OUT OF MEMORY";
            case EResultCode::InternalError:    return "INTERNAL ERROR";
        }

        return "UNKNOWN RESULT CODE";
    }

    inline EResultCode StringToResultCode(const std::string& a_string) {
        if (a_string == "SUCCESS")              { return EResultCode::Success;          }
        if (a_string == "NOT INITIALIZED")      { return EResultCode::NotInitialized;   }
        if (a_string == "INVALID PARAMETER(S)") { return EResultCode::InvalidParameter; }
        if (a_string == "MODEL LOAD FAILED")    { return EResultCode::ModelLoadFailed;  }
        if (a_string == "PROCESSING FAILED")    { return EResultCode::ProcessingFailed; }
        if (a_string == "CANCELLED")            { return EResultCode::Cancelled;        }
        if (a_string == "OUT OF MEMORY")        { return EResultCode::OutOfMemory;      }
        if (a_string == "INTERNAL ERROR")       { return EResultCode::InternalError;    }
        
        return EResultCode::Unknown;
    }
    
}

#endif