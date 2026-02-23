#pragma once

#include "FileConfigs.h"
#include "HardwareConfigs.h"
#include "SpeechSynthesisConfigs.h"

namespace VoxBox {
    // Top-level VoxBox-Piper config struct
    struct VB_TTS_API SVBPiperConfig {
        SDependencyConfig m_dependencies;
        SVoiceModelConfig m_voice_config;
        SSynthesisConfig m_synthesis;
        SHardwareConfig m_acceleration;
    };
}