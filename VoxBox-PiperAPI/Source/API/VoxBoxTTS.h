#pragma once
#ifdef _WIN32
    #ifdef VB_EXPORT_TTS
        #define VB_TTS_API __declspec(dllexport)
    #else
        #define VB_TTS_API __declspec(dllimport)
    #endif
#else
    #define VB_EXPORT_TTS
    #ifndef __stdcall
        #define __stdcall
    #endif
#endif


#ifdef __cplusplus
    #include <cstdint>
#else
    #include <stdint.h>
#endif


#ifdef __cplusplus
extern "C" {
#endif

    /* Setup & Shutdown helpers */
    VB_TTS_API void __stdcall VB_TTS_Init(const char* a_model_onnx_path, const char* a_model_json_path);
    VB_TTS_API void __stdcall VB_TTS_Reinit(const char* a_model_onnx_path, const char* a_model_json_path);
    VB_TTS_API void __stdcall VB_TTS_Shutdown();


    /* Garbage collection */
    // Frees the pointer to samples created via VB_TTS_ToRaw().
    VB_TTS_API void __stdcall VB_TTS_FreeRaw(const int16_t* a_samples);


    /* Utility functions */
    //  Note: these functions assume initialization was already performed and completed
    VB_TTS_API void __stdcall VB_TTS_StreamToStreamRaw(const void* a_input_stream, const void* a_output_stream);

    // Caller takes ownership of return value
    // Returned object should be freed via VB_TTS_FreeRaw()
    VB_TTS_API int16_t* __stdcall VB_TTS_ToRaw(char const* const a_text, int* const a_out_sample_count);
    
    VB_TTS_API void __stdcall VB_TTS_ToWAVFile(const char* a_text, const char* a_wav_path);

#ifdef __cplusplus
}
#endif
