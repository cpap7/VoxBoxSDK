#pragma once
#include "HardwareConfigs.h"
#include "FileConfigs.h"
#include "AudioConfigs.h"

//#include <string>

namespace VoxBox {

    // Advanced feature flags
    struct VB_STT_API SFeatureConfig {
        std::string m_suppress_regex;      // Regex for token suppression
        std::string m_tdrz_speaker_turn = " [SPEAKER_TURN]";

        bool m_diarize          = false;    // Stereo speaker diarization
        bool m_tinydiarize      = false;    // Model-based diarization (tdrz)
        bool m_detect_language  = false;    // Auto-detect language only
        bool m_suppress_blank   = true;     // Suppress blank outputs
        bool m_suppress_nst     = true;     // Suppress non-speech tokens
        bool m_flash_attn       = false;    // Use flash attention
        bool m_debug_mode       = false;    // Enable debug output
    };

    // Top-level struct containing all configs
	struct VB_STT_API SVBWhisperConfig {
		SModelConfig            m_model_config;
        SGrammarConfig          m_grammer_config;
        SFeatureConfig          m_feature_config;
        SDTWConfig              m_dtw_config;
        SSystemConfig           m_system_config;
        SOpenVINOConfig         m_open_vino_config;
        SDecoderConfig          m_decoder_config;
        SAudioProcessingConfig  m_audio_processing_config;
        SThresholdConfig        m_threshold_config;
        STemperatureConfig      m_temperature_config;
		SHardwareConfig         m_hardware_config;
        SOutputConfig           m_output_config;
	};
}