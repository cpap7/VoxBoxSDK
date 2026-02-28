#include "vbwpch.h"
#include "VoxBoxSTT.h"
#include "../Core/STTEngine.h"

#include <whisper.h>

// Helper function C-C++ interop
// Converts the C-equivalent config to the native C++ version
static VoxBox::SSTTConfig ConvertSSTConfig(const VB_STT_Config_t* a_config) {
    VoxBox::SSTTConfig config;

    if (a_config) {
        // TODO: Add more C-config options and convert them here

        // Model config
        config.m_model_config.m_model_path           = a_config->m_model_path ? a_config->m_model_path : "";
        config.m_model_config.m_initial_prompt       = a_config->m_initial_prompt ? a_config->m_initial_prompt : "";
        config.m_model_config.m_language             = a_config->m_language ? a_config->m_language : "en"; // English = default
        config.m_model_config.m_translate_to_english = a_config->m_translate_to_english ? a_config->m_translate_to_english : false; 

        // Hardware config
        config.m_hardware_config.m_thread_count = a_config->m_thread_count ? a_config->m_thread_count : 0; // Auto detect if not specified
        config.m_hardware_config.m_use_gpu      = a_config->m_use_gpu ? a_config->m_use_gpu : true;
    }

    return config;
}

VB_STT_API VB_STT_EngineHandle_t VB_CALL VB_STT_Create(const VB_STT_Config_t* a_config) {
    if (!a_config) {
        printf("[VoxBox] STT Error: Invalid config\n");
        return nullptr;
    }
    
    if (!a_config->m_model_path) {
        printf("[VoxBox] STT Error: Missing model path\n");
        return nullptr;
    }

    try {
        VoxBox::SSTTConfig config = ConvertSSTConfig(a_config);
        auto* stt_engine = new VoxBox::CCoreSTTEngine(config);
        return reinterpret_cast<VB_STT_EngineHandle_t>(stt_engine);
    }
    catch (...) {
        return nullptr;
    }


}

VB_STT_API void VB_CALL VB_STT_Destroy(VB_STT_EngineHandle_t a_engine) {
    if (a_engine) {
        auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
        delete stt_engine;
    }
}

VB_STT_API int VB_CALL VB_STT_IsLoaded(VB_STT_EngineHandle_t a_engine) {
    if (!a_engine) {
        return 0;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    return stt_engine != nullptr ? 1 : 0; // Check context initialization first 
}

VB_STT_API VB_STT_Result_t VB_CALL VB_STT_Transcribe(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count) {
    VB_STT_Result_t c_transcript_result = {
        nullptr,    // m_text
        nullptr,    // m_word_probabilities
        0,          // m_word_count
        false       // m_success
    };

    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return c_transcript_result;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    try {
        VoxBox::STranscriptResult transcript_result = stt_engine->Transcribe(a_audio, a_sample_count, true);
        if (!transcript_result.m_success) {
            return c_transcript_result;
        }

        // Allocate memory & copy text
        c_transcript_result.m_text = static_cast<char*>(malloc(transcript_result.m_text.size() + 1));
        if (c_transcript_result.m_text) {
            std::memcpy(c_transcript_result.m_text, transcript_result.m_text.c_str(), transcript_result.m_text.size() + 1);
        }

        // Allocate memory & copy word probabilities
        c_transcript_result.m_word_count = static_cast<int>(transcript_result.m_word_probabilities.size());
        if (c_transcript_result.m_word_count > 0) {
            size_t probabilities_size = c_transcript_result.m_word_count * sizeof(float);
            c_transcript_result.m_word_probabilities = static_cast<float*>(malloc(probabilities_size));

            if (c_transcript_result.m_word_probabilities) {
                std::memcpy(c_transcript_result.m_word_probabilities, transcript_result.m_word_probabilities.data(), probabilities_size);
            }
        }

        c_transcript_result.m_success = true;
    }
    catch (...) {
        c_transcript_result.m_success = false;
    }

    return c_transcript_result;
}

VB_STT_API const char* VB_CALL VB_STT_TranscribeSimple(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count) {
    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return nullptr;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    try {
        VoxBox::STranscriptResult transcript_result = stt_engine->Transcribe(a_audio, a_sample_count, true);
        if (!transcript_result.m_success || transcript_result.m_text.empty()) {
            return nullptr;
        }
        
        char* transcript_text = static_cast<char*>(malloc(transcript_result.m_text.size() + 1));
        if (transcript_text) {
            std::memcpy(transcript_text, transcript_result.m_text.c_str(), transcript_result.m_text.size() + 1);
        }

        return transcript_text;
    }
    catch (...) {
        return nullptr;
    }
}

VB_STT_API const char* VB_CALL VB_STT_DetectLanguage(VB_STT_EngineHandle_t a_engine, const float* a_audio, int a_sample_count) {
    if (!a_engine || !a_audio || a_sample_count <= 0) {
        return nullptr;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    
    VoxBox::SLanguageResult language_result = stt_engine->DetectLanguage(a_audio, a_sample_count);
    if (language_result.m_language_code.empty()) {
        return nullptr;
    }

    try {
        char* language_string = static_cast<char*>(malloc(language_result.m_language_code.size() + 1));
        if (language_string) {
            std::memcpy(language_string, language_result.m_language_code.c_str(), language_result.m_language_code.size() + 1);
        }

        return language_string;
    }
    catch (...) {
        return nullptr;
    }
}

VB_STT_API void VB_CALL VB_STT_SetProgressCallback(VB_STT_EngineHandle_t a_engine, void (*a_callback)(int a_progress)) {
    if (!a_engine) {
        return;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    stt_engine->SetProgressCallback(a_callback);
}

VB_STT_API void VB_CALL VB_STT_Cancel(VB_STT_EngineHandle_t a_engine) {
    if (!a_engine) {
        return;
    }

    auto* stt_engine = reinterpret_cast<VoxBox::CCoreSTTEngine*>(a_engine);
    stt_engine->Cancel();
}

VB_STT_API void VB_CALL VB_STT_FreeString(const char* a_string) {
    if (a_string) {
        free(const_cast<char*>(a_string));
    }
}

VB_STT_API void VB_CALL VB_STT_FreeFloatArray(float* a_float_array) {
    if (a_float_array) {
        free(a_float_array);
    }
}

VB_STT_API void VB_CALL VB_STT_FreeResult(VB_STT_Result_t* a_result) {
    if (a_result) {
        if (a_result->m_text) {
            free(a_result->m_text);
            a_result->m_text = nullptr;
        }

        if (a_result->m_word_probabilities) {
            free(a_result->m_word_probabilities);
            a_result->m_word_probabilities = nullptr;
        }

        a_result->m_word_count  = 0;
        a_result->m_success     = false;
    }
}

VB_STT_API VB_STT_Config_t VB_CALL VB_STT_GetDefaultConfig() {
    VB_STT_Config_t config = {};
    // TODO: Add more config options and update them here
    config.m_model_path             = nullptr;
    config.m_language               = "en"; // English = default
    config.m_thread_count           = 0;    // 0 = auto detect via whisper.cpp 
    config.m_use_gpu                = true;
    config.m_translate_to_english   = false;
 
    return config;
}

VB_STT_API const char* VB_CALL VB_STT_GetVersion() {
    return VB_STT_VERSION;
}

// C++ API
#ifdef __cplusplus
namespace VoxBox {
    CVBSTTEngine::CVBSTTEngine(const SSTTConfig& a_config) {
        m_engine = std::make_unique<CCoreSTTEngine>(a_config);
	}
	
    CVBSTTEngine::~CVBSTTEngine() = default; // We define this here for visibility

    CVBSTTEngine::CVBSTTEngine(CVBSTTEngine&& a_other) noexcept 
        : m_engine(std::move(a_other.m_engine)){
	}

	void CVBSTTEngine::operator=(CVBSTTEngine&& a_other) noexcept {
        m_engine = std::move(a_other.m_engine);
	}

    bool CVBSTTEngine::IsLoaded() const {
        return m_engine->IsLoaded();
    }

    SVBTranscriptResult CVBSTTEngine::Transcribe(const std::vector<float>& a_audio_data) {
        return Transcribe(a_audio_data.data(), static_cast<int>(a_audio_data.size()));
    }

    SVBTranscriptResult CVBSTTEngine::Transcribe(const float* a_audio_data, int a_sample_count) {
        SVBTranscriptResult result;
        if (!m_engine) {
            result.m_result_code = EResultCode::NotInitialized;
            return result;
        }
        
        if (!a_audio_data || a_sample_count <= 0) {
            result.m_result_code = EResultCode::InvalidParameter;
            return result;
        }

        try {
            STranscriptResult core_result = m_engine->Transcribe(a_audio_data, a_sample_count, true);
            if (core_result.m_success) {
                result.m_text               = std::move(core_result.m_text);
                result.m_word_probabilities = std::move(core_result.m_word_probabilities);
                result.m_part_indices       = std::move(core_result.m_part_indices);
                result.m_result_code        = EResultCode::Success;
            }
            else {
                result.m_result_code        = EResultCode::ProcessingFailed;
            }
        }
        catch(...) {
            result.m_result_code = EResultCode::InternalError;
        }
        
        return result;
    }

    std::string CVBSTTEngine::TranscribeSimple(const float* a_audio_data, int a_sample_count) {
        if (!m_engine || !a_audio_data || a_sample_count <= 0) {
            return "";
        }

        try {
            STranscriptResult core_result = m_engine->Transcribe(a_audio_data, a_sample_count, false);
            return core_result.m_success ? core_result.m_text : "";
        }
        catch (...) {
            return "";
        }
    }

    SVBLanguageResult CVBSTTEngine::DetectLanguage(const float* a_audio_data, int a_sample_count) {
        SVBLanguageResult result;
        if (!m_engine) {
            result.m_result_code = EResultCode::NotInitialized;
            return result;
        }

        if (!a_audio_data || a_sample_count <= 0) {
            result.m_result_code = EResultCode::InvalidParameter;
            return result;
        }

        try {
            SLanguageResult core_result = m_engine->DetectLanguage(a_audio_data, a_sample_count);
            if (!core_result.m_language_code.empty()) {
                result.m_language_code = std::move(core_result.m_language_code);
                result.m_confidence    = core_result.m_confidence;
                result.m_result_code   = EResultCode::Success;
            }
            else {
                result.m_result_code   = EResultCode::ProcessingFailed;
            }
        }
        catch (...) {
            result.m_result_code = EResultCode::InternalError;
        }


        return result;
    }

    void CVBSTTEngine::SetProgressCallback(ProgressCallbackFn a_callback) {
        if (!m_engine) {
            return;
        }

        m_engine->SetProgressCallback(a_callback);
    }

    void CVBSTTEngine::Cancel() {
        if (!m_engine) {
            return;
        }
        
        m_engine->Cancel();
    }

    std::unique_ptr<CVBSTTEngine> CVBSTTEngine::Create(const SSTTConfig& a_config) {
        try {
            return std::make_unique<CVBSTTEngine>(a_config);
        }
        catch (...) {
            return nullptr;
        }
    }

    SSTTConfig CVBSTTEngine::GetDefaultConfig() {
        return SSTTConfig();
    }

    std::string CVBSTTEngine::GetVersion() {
        return VB_STT_VERSION;
    }
}
#endif