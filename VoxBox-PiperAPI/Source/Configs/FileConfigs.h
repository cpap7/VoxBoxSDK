#pragma once

//#include <filesystem>
//#include <optional>

namespace VoxBox {
    // Voice model paths
    struct SVoiceModelConfig {
        std::filesystem::path m_model_path;                           // Path to ONNX voice file (see https://github.com/mush42/libtashkeel/) 
        std::filesystem::path m_model_config_path;                    // Path to JSON voice config file
    };

    // External dependency paths
    struct SDependencyConfig {
        std::optional<std::filesystem::path> m_espeak_data_path;      // espeak-ng data
        std::optional<std::filesystem::path> m_tashkeel_model_path;   // libtashkeel ORT model
    };
}