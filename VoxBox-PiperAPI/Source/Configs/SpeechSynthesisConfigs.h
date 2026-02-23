#pragma once

namespace VoxBox {
    // Audio synthesis parameters
    struct VB_TTS_API SSynthesisConfig {
        std::optional<float> m_noise_scale = 0.667f;                 // Audio generator noise (default = 0.667f)
        std::optional<float> m_length_scale = 1.0f;                  // Speaking speed (<1.0f = faster, >1.0f = slower, 1.0f == normal/default)
        std::optional<float> m_noise_width = 0.8f;                   // Phoneme width noise variation (default = 0.8f)
        std::optional<float> m_sentence_silence_seconds = 0.2f;      // Silence after each sentence (default = 0.2f)
    };
}