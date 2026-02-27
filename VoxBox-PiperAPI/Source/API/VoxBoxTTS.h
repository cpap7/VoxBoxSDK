#pragma once
// This include is fragile but it's fine for now
#include "../../VoxBox-Common/Source/VBCommon.h"  // TODO: adjust w/ releases and/or fix premake files

#ifdef __cplusplus
    #include <cstdint>
    #include <vector>
    #include <memory>
    #include <utility>
    #include <functional>
    #include <optional>
    #include <filesystem>
    #include "../Core/TTSConfig.h"

#else
    #include <stdint.h>
   //#include <stddef.h>
   //#include <stdbool.h>
#endif

#define VB_TTS_VERSION "1.0.0"

// C API
#ifdef __cplusplus
extern "C" {
#endif

    /* FOR INTEROP WITH C++ */

    typedef struct VB_TTS_Handle_t* VB_TTS_EngineHandle_t; // Opaque handle for C

    // C config struct 
    typedef struct {
        const char* m_model_onnx_path;
        const char* m_model_onnx_json_path; 
        
        float m_length_scale;       // 1.0 = normal speed
        float m_noise_scale;        // 0.667 = default
        float m_noise_width;        // 0.8 = default

        int m_speaker_id;           // -1 = default
        int m_sample_rate;          // 0 for model default
    } VB_TTS_Config_t;

    // C audio result struct
    typedef struct {
        int m_sample_count;
        int m_sample_rate;
        int m_success;

        int16_t* m_samples;         // Caller must be free via VB_TTS_FreeSamples
    } VB_TTS_AudioResult_t;


    // Lifecycle
    VB_TTS_API VB_TTS_EngineHandle_t VB_CALL VB_TTS_Create(const VB_TTS_Config_t* a_config);
    VB_TTS_API void VB_CALL VB_TTS_Destroy(VB_TTS_EngineHandle_t a_engine);
    VB_TTS_API int VB_CALL VB_TTS_IsLoaded(VB_TTS_EngineHandle_t a_engine);
    
    // Speech Synthesis
    VB_TTS_API VB_TTS_AudioResult_t VB_CALL VB_TTS_Synthesize(VB_TTS_EngineHandle_t a_engine, const char* a_text);
    VB_TTS_API int16_t* VB_CALL VB_TTS_SynthesizeSimple(VB_TTS_EngineHandle_t a_engine, const char* a_text, int* a_out_sample_count);
    VB_TTS_API int VB_CALL VB_TTS_SynthesizeToWAVFile(VB_TTS_EngineHandle_t a_engine, const char* a_text, const char* a_wav_file_path);

    // Control callbacks/cancel synthesis
    VB_TTS_API void VB_CALL VB_TTS_SetProgressCallback(VB_TTS_EngineHandle_t a_engine, void (*a_callback)(int a_progress));
    VB_TTS_API void VB_CALL VB_TTS_Cancel(VB_TTS_EngineHandle_t a_engine);

    // Voice settings (runtime adjustable)
    VB_TTS_API void VB_CALL VB_TTS_SetSpeaker(VB_TTS_EngineHandle_t a_engine, int a_speaker_id);
    VB_TTS_API void VB_CALL VB_TTS_SetSpeed(VB_TTS_EngineHandle_t a_engine, float a_length_scale);
    
    // Memory management
    VB_TTS_API void VB_CALL VB_TTS_FreeSamples(int16_t* a_samples);
    VB_TTS_API void VB_CALL VB_TTS_FreeResult(VB_TTS_AudioResult_t* a_result);

    // Utility
    VB_TTS_API VB_TTS_Config_t VB_CALL VB_TTS_GetDefaultConfig();
    VB_TTS_API const char* VB_CALL VB_TTS_GetVersion();

#ifdef __cplusplus
}
#endif


#ifdef __cplusplus
namespace VoxBox {
    // Forward declarations
    class CCoreTTSEngine;
    //struct STTSConfig;

    // Wrapper for audio buffer
    // Used primarily by the engine wrapper class 
    // Used to store values produced by the core audio buffer class
    class VB_TTS_API CVBTTSAudioBuffer {
        // TODO: Consolidate this class better w/ the core class
        // Or remove it entirely
    private:
        std::vector<int16_t> m_samples;
        int m_sample_rate = 22050; 

    public:
        CVBTTSAudioBuffer() = default;
        CVBTTSAudioBuffer(std::vector<int16_t> a_samples, int a_sample_rate);
        ~CVBTTSAudioBuffer();

        // Accessors
        inline const int16_t* Data() const { return m_samples.data();                   }
        inline int SampleCount() const     { return static_cast<int>(m_samples.size()); }
        inline int SampleRate() const      { return m_sample_rate;                      }
        inline bool IsEmpty() const        { return m_samples.empty();                  }

        // Export
        bool SaveToWAVFile(const std::string& path) const;
        inline std::vector<int16_t> Release() { return std::move(m_samples);            }
    };

    // Result type
    struct VB_TTS_API SSynthesisResult {
        CVBTTSAudioBuffer m_audio_buffer;
        EResultCode m_result_code = EResultCode::NotInitialized;

        inline bool Success() const     { return m_result_code == EResultCode::Success; }
        inline operator bool() const    { return Success();                             }
    };

    // TTS engine API wrapper
    // Serves to dispatch calls to the core engine class
    // and has TTS engine class factory functions
    class VB_TTS_API CVBTTSEngine {
    private:
        std::unique_ptr<CCoreTTSEngine> m_engine = nullptr;

    public:
        CVBTTSEngine(const STTSConfig& config);
        ~CVBTTSEngine();
        
        // Don't need copies
        CVBTTSEngine(const CVBTTSEngine&) = delete;
        CVBTTSEngine& operator=(const CVBTTSEngine&) = delete;
        
        CVBTTSEngine(CVBTTSEngine&& a_other) noexcept;
        void operator=(CVBTTSEngine&& a_other) noexcept;

        // Status
        bool IsLoaded() const;
        explicit operator bool() const { return IsLoaded(); }

        // Synthesis
        SSynthesisResult Synthesize(const std::string& a_text);
        CVBTTSAudioBuffer SynthesizeSimple(const std::string& a_text);
        bool SynthesizeToWAVFile(const std::string& a_text, const std::string& a_wav_path);

        // Voice settings
        void SetSpeakerID(int a_speaker_id);
        void SetSpeed(float a_length_scale);
        void SetNoiseScale(float a_noise_scale);
        
        // Control
        void SetProgressCallback(ProgressCallbackFn a_callback_function);
        void Cancel();

       
        
        // Factory
        static std::unique_ptr<CVBTTSEngine> Create(const STTSConfig& a_config);

        // Misc
        static const char* GetVersion();
        static STTSConfig GetDefaultConfig();
    };
}

#endif
