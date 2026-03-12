#pragma once

#include <optional>
#include <filesystem>

namespace VoxBox {
    // Voice model paths
    struct STTSModelConfig {
        std::optional<std::filesystem::path> m_tashkeel_model_path;   // Path to libtashkeel ORT model

        std::filesystem::path m_model_onnx_path;                      // Path to ONNX voice file (see https://github.com/mush42/libtashkeel/) 
        std::filesystem::path m_model_onnx_json_path;                 // Path to JSON voice config file
        std::filesystem::path m_espeak_data_path;                     // Path to espeak-ng-data folder

        bool m_use_espeak = true;
        bool m_use_tashkeel = false;
    };

    // Audio synthesis parameters
    struct SSynthesisConfig {
        std::optional<float> m_noise_scale = 0.667f;                 // Audio generator noise (default = 0.667f)
        std::optional<float> m_length_scale = 1.0f;                  // Speaking speed (<1.0f = faster, >1.0f = slower, 1.0f == normal/default)
        std::optional<float> m_noise_width = 0.8f;                   // Phoneme width noise variation (default = 0.8f)
        std::optional<float> m_sentence_silence_seconds = 0.2f;      // Silence after each sentence (default = 0.2f)
    };

    // Hardware
    // Note: Could possibly use a boolean
    enum class EGPUBackendType : uint8_t {
        None = 0,

        // GPU acceleration options
        // TODO: Investigate other GPU acceleration options included for Piper
        Cuda = 1,

        Default = None
    };

    struct STTSHardwareConfig {
        EGPUBackendType m_gpu_acceleration_type = EGPUBackendType::Default;
    };

    //////////////////////////////////////////////////////

    // Top-level config struct for interfacing with Piper
    // Used by the API class CTTSEngine (see VoxBoxTTS.h) 
    // and the internal core class (see TTSEngine.h)
    struct VB_TTS_API STTSConfig {
        STTSModelConfig     m_voice_config;
        SSynthesisConfig    m_synthesis_config;
        STTSHardwareConfig  m_hardware_config;
    };
}