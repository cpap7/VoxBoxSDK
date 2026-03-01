#pragma once

#ifdef __cplusplus
    #include <string>
    #include <vector>
    #include <memory>
    #include <functional>

    #include "../Core/STTConfig.h"
    #include "../Core/STTResults.h"
#endif

#define VB_STT_VERSION "1.0.0"

// C API
#ifdef __cplusplus
extern "C" {
#endif
    // Opaque handle for C
    typedef struct VB_STT_Handle_t* VB_STT_EngineHandle_t;

    // C-specific config struct (for interop)
    typedef struct {
        const char* m_model_path;
        const char* m_initial_prompt;
        const char* m_language;            // nullptr for auto-detect
        int m_thread_count;                // 0 for auto
        bool m_use_gpu;                    // Default = true
        bool m_translate_to_english;       // Default = false
    } VB_STT_Config_t;

    // Transcript result struct (for interop)
    typedef struct {
        char* m_text;                     // Caller must free via VB_STT_FreeString
        float* m_word_probabilities;      // Caller must free via VB_STT_FreeFloatArray
        int m_word_count;
        bool m_success;
    } VB_STT_Result_t;

    // Lifecycle
    VB_STT_API VB_STT_EngineHandle_t VB_CALL VB_STT_Create(const VB_STT_Config_t* a_config);
    VB_STT_API void VB_CALL VB_STT_Destroy(VB_STT_EngineHandle_t a_engine);
    VB_STT_API int VB_CALL VB_STT_IsLoaded(VB_STT_EngineHandle_t a_engine);

    // Transcription
    VB_STT_API VB_STT_Result_t VB_CALL VB_STT_Transcribe(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);
    VB_STT_API const char* VB_CALL VB_STT_TranscribeSimple(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);
    VB_STT_API const char* VB_CALL VB_STT_DetectLanguage(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count);

    // Control
    VB_STT_API void VB_CALL VB_STT_SetProgressCallback(VB_STT_EngineHandle_t a_engine, void (*a_callback)(int a_progress));
    VB_STT_API void VB_CALL VB_STT_Cancel(VB_STT_EngineHandle_t a_engine);

    // Memory management
    VB_STT_API void VB_CALL VB_STT_FreeString(const char* a_string);
    VB_STT_API void VB_CALL VB_STT_FreeFloatArray(float* a_float_array);
    VB_STT_API void VB_CALL VB_STT_FreeResult(VB_STT_Result_t* a_result);

    // Utility
    VB_STT_API VB_STT_Config_t VB_CALL VB_STT_GetDefaultConfig();
    VB_STT_API const char* VB_CALL VB_STT_GetVersion();

#ifdef __cplusplus
}
#endif

// C++ API
#ifdef __cplusplus

namespace VoxBox {
    // Forward declaration
    class CSTTEngineImpl;

    // C++ wrapper class for the core engine class
    // Serves as an API call dispatcher & has its own factory
    class VB_STT_API CSTTEngine {
    private:
        std::unique_ptr<CSTTEngineImpl> m_engine = nullptr; // Instance

    public:
        // Lifecycle
        CSTTEngine(const SSTTConfig& config);
        ~CSTTEngine();

        // Don't need copies
        CSTTEngine(const CSTTEngine&) = delete;
        void operator=(const CSTTEngine&) = delete;
        
        CSTTEngine(CSTTEngine&& a_other) noexcept;
        void operator=(CSTTEngine&& a_other) noexcept;

        // Status
        bool IsLoaded() const;
        explicit operator bool() const { return IsLoaded(); }

        // Transcription
        STranscriptResult Transcribe(const std::vector<float>& a_audio_data);
        STranscriptResult Transcribe(const float* a_audio_data, int a_sample_count);
        std::string TranscribeSimple(const float* a_audio_data, int a_sample_count);

        // Language detection
        SLanguageResult DetectLanguage(const float* a_audio_data, int a_sample_count);

        // Control
        void SetProgressCallback(ProgressCallbackFn a_callback_function);
        void Cancel();

        // Factory
        static std::unique_ptr<CSTTEngine> Create(const SSTTConfig& a_config);
        
        // Misc
        static SSTTConfig GetDefaultConfig();
        static std::string GetVersion();
    };

} // namespace VoxBox
#endif