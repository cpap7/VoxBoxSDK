#pragma once
#include <VBCommon.h>

#ifdef __cplusplus
    #include <cstdint>
    #include <vector>
    #include <memory>
    #include <utility>
    #include <functional>
    #include <optional>
    #include <filesystem>
    #include <atomic>
    
    #include "../Core/TTSConfig.h"
    #include "../Core/TTSResult.h"
#else
    #include <stdint.h>
#endif

#define VB_TTS_VERSION "1.0.0"

// C API
#ifdef __cplusplus
extern "C" {
#endif

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
    class CTTSEngineImpl;

    // TTS engine API wrapper
    // Serves to dispatch calls to the core engine class
    // and has its own factory function
    class VB_TTS_API CTTSEngine {
    private:
        std::unique_ptr<CTTSEngineImpl> m_engine = nullptr;

    public:
        CTTSEngine(const STTSConfig& config);
        ~CTTSEngine();
        
        // Don't need copies
        CTTSEngine(const CTTSEngine&) = delete;
        void operator=(const CTTSEngine&) = delete;
        
        CTTSEngine(CTTSEngine&& a_other) noexcept;
        void operator=(CTTSEngine&& a_other) noexcept;

        // Status
        bool IsLoaded() const;
        explicit operator bool() const { return IsLoaded(); }

        // Synthesis
        SAudioResult Synthesize(const std::string& a_text);
        bool SynthesizeToWAVFile(const std::string& a_text, const std::string& a_wav_path);

        // Voice settings
        void SetSpeakerID(int a_speaker_id);
        void SetSpeed(float a_length_scale);
        void SetNoiseScale(float a_noise_scale);

        // Factory
        static std::unique_ptr<CTTSEngine> Create(const STTSConfig& a_config);

        // Misc
        static const char* GetVersion();
        static STTSConfig GetDefaultConfig();
    };
} // namespace VoxBox
#endif
