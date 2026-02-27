#include "vbppch.h"
#include "VoxBoxTTS.h"

#include "../Core/TTSEngine.h"

#include <piper.hpp>


// Helper function for C-C++ interop
// Converts the C config to the native C++ version
static VoxBox::STTSConfig ConvertConfig(const VB_TTS_Config_t* a_config) {
	VoxBox::STTSConfig config;
	
	if (a_config) {
		// Model path settings
		config.m_voice_config.m_model_onnx_path			= a_config->m_model_onnx_path ? a_config->m_model_onnx_path : "";
		config.m_voice_config.m_model_onnx_json_path	= a_config->m_model_onnx_json_path ? a_config->m_model_onnx_json_path : "";
	
		// Audio settings
		config.m_synthesis_config.m_length_scale		= a_config->m_length_scale;
		config.m_synthesis_config.m_noise_scale			= a_config->m_noise_scale;
		config.m_synthesis_config.m_noise_width			= a_config->m_noise_width;
	}

	return config;
}

VB_TTS_API VB_TTS_EngineHandle_t VB_CALL VB_TTS_Create(const VB_TTS_Config_t* a_config) {
	if (!a_config || !a_config->m_model_onnx_path || !a_config->m_model_onnx_json_path) {
		printf("[VB TTS] Error: Missing model or model json path.\n");
		return nullptr;
	}

	try {
		VoxBox::STTSConfig config = ConvertConfig(a_config);
		auto* tts_engine = new VoxBox::CCoreTTSEngine(config);
		return reinterpret_cast<VB_TTS_EngineHandle_t>(tts_engine);
	}
	catch (...) {
		return nullptr;
	}

}

VB_TTS_API void VB_CALL VB_TTS_Destroy(VB_TTS_EngineHandle_t a_engine) {
	if (a_engine) {
		auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
		delete engine;
	}
}

VB_TTS_API int VB_CALL VB_TTS_IsLoaded(VB_TTS_EngineHandle_t a_engine) {
	if (!a_engine) {
		return 0;
	}

	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	return engine->IsInitialized() ? 1 : 0;
}

VB_TTS_API VB_TTS_AudioResult_t VB_CALL VB_TTS_Synthesize(VB_TTS_EngineHandle_t a_engine, const char* a_text) {
	VB_TTS_AudioResult_t result = { 
		0,			// Sample count
		0,			// Sample rate
		0,			// Success value (bool internally)
		nullptr		// 2 byte int pointer for the samples
	};

	if (!a_engine || !a_text) {
		return result;
	}

	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	if (!engine->IsInitialized()) {
		return result;
	}

	try {
		std::vector<int16_t> audio = engine->Synthesize(a_text);
		if (audio.empty()) {
			return result;
		}

		result.m_sample_count	= static_cast<int>(audio.size());
		result.m_sample_rate	= engine->GetSampleRate();
		result.m_samples		= static_cast<int16_t*>(malloc(audio.size() * sizeof(int16_t)));

		if (result.m_samples) {
			std::memcpy(result.m_samples, audio.data(), audio.size() * sizeof(int16_t));
			result.m_success = 1; // Successful synthesis
		}

	}
	catch (...) {
		result.m_success = 0; // Failed synthesis
	}

 	return result;
}

VB_TTS_API int16_t* VB_CALL VB_TTS_SynthesizeSimple(VB_TTS_EngineHandle_t a_engine, const char* a_text, int* a_out_sample_count) {
	if (!a_engine || !a_text || !a_out_sample_count) {
		return nullptr;
	}
	
	*a_out_sample_count = 0;
	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	if (!engine->IsInitialized()) {
		return nullptr;
	}


	try {
		std::vector<int16_t> audio = engine->Synthesize(a_text);
	
		if (audio.empty()) {
			return nullptr;
		}

		*a_out_sample_count = static_cast<int>(audio.size());
		int16_t* samples = static_cast<int16_t*>(malloc(audio.size() * sizeof(int16_t)));

		if (samples) {
			std::memcpy(samples, audio.data(), audio.size() * sizeof(int16_t));
		}

		return samples;

	}
	catch (...) {
		return nullptr;
	}


}

VB_TTS_API int VB_CALL VB_TTS_SynthesizeToWAVFile(VB_TTS_EngineHandle_t a_engine, const char* a_text, const char* a_wav_file_path) {
	if (!a_engine || !a_text || !a_wav_file_path) {
		return 0;
	}

	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	if (!engine->IsInitialized()) {
		return 0;
	}

	try {
		engine->SynthesizeToWAVFile(a_text, a_wav_file_path);
		return 1;
	}
	catch (...) {
		return 0;
	}
}

VB_TTS_API void VB_CALL VB_TTS_SetProgressCallback(VB_TTS_EngineHandle_t a_engine, void (*a_callback)(int a_progress)) {
	// TODO: implement once its within the core engine as a feature
	(void)a_engine;
	(void)a_callback;
}

VB_TTS_API void VB_CALL VB_TTS_Cancel(VB_TTS_EngineHandle_t a_engine) {
	// TODO: implement once its within the core engine as a feature
	(void)a_engine;
}

VB_TTS_API void VB_CALL VB_TTS_SetSpeaker(VB_TTS_EngineHandle_t a_engine, int a_speaker_id) {
	if (!a_engine) {
		return;
	}

	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	if (!engine->IsInitialized()) {
		return;
	}

	printf("[VoxBox] Setting speaker ID to: %d\n", a_speaker_id);
	engine->SetSpeakerID(a_speaker_id);
}

VB_TTS_API void VB_CALL VB_TTS_SetSpeed(VB_TTS_EngineHandle_t a_engine, float a_length_scale) {
	if (!a_engine) {
		return;
	}

	auto* engine = reinterpret_cast<VoxBox::CCoreTTSEngine*>(a_engine);
	if (!engine->IsInitialized()) {
		return;
	}

	VoxBox::SSynthesisConfig config = engine->GetSynthesisConfig();
	config.m_length_scale = a_length_scale;

	printf("[VoxBox] Setting speech synthesis length scale to: %.2f\n", a_length_scale);
	engine->SetSynthesisConfig(config);
}

VB_TTS_API void VB_CALL VB_TTS_FreeSamples(int16_t* a_samples) {
	if (a_samples) {
		free(a_samples);
		//a_samples = nullptr;
	}
}

VB_TTS_API void VB_CALL VB_TTS_FreeResult(VB_TTS_AudioResult_t* a_result) {
	if (a_result && a_result->m_samples) {
		free(a_result->m_samples);
		a_result->m_samples			= nullptr;
		a_result->m_sample_count	= 0;
		a_result->m_sample_rate		= 0;
		a_result->m_success			= 0;
	}
}

VB_TTS_API VB_TTS_Config_t VB_CALL VB_TTS_GetDefaultConfig() {
	VB_TTS_Config_t config = {};
	config.m_model_onnx_path		= nullptr;
	config.m_model_onnx_json_path	= nullptr;
	
	config.m_length_scale			= 1.0f;
	config.m_noise_scale			= 0.667f;
	config.m_noise_width			= 0.8f;
	
	config.m_speaker_id				= -1;
	config.m_sample_rate			= 0;


	return config;
}

VB_TTS_API const char* VB_CALL VB_TTS_GetVersion() {
	return VB_TTS_VERSION;
}

// C++ API
#ifdef __cplusplus
namespace VoxBox {
	// TTS Audio Buffer

	CVBTTSAudioBuffer::CVBTTSAudioBuffer(std::vector<int16_t> a_samples, int a_sample_rate) 
		: m_samples(std::move(a_samples)), m_sample_rate(a_sample_rate) {
	}

	CVBTTSAudioBuffer::~CVBTTSAudioBuffer() = default;

	bool CVBTTSAudioBuffer::SaveToWAVFile(const std::string& a_path) const {
		// TODO
		return false;
	}


	// TTS Engine

	CVBTTSEngine::CVBTTSEngine(const STTSConfig& a_config) {
		m_engine = std::make_unique<CCoreTTSEngine>(a_config);
	}

	CVBTTSEngine::~CVBTTSEngine() = default;
	
	CVBTTSEngine::CVBTTSEngine(CVBTTSEngine&& a_other) noexcept 
		: m_engine(std::move(a_other.m_engine)) {
	
	}

	void CVBTTSEngine::operator=(CVBTTSEngine&& a_other) noexcept {
		m_engine = std::move(a_other.m_engine);
	}


	bool CVBTTSEngine::IsLoaded() const {
		return m_engine->IsInitialized();
	}

	SSynthesisResult CVBTTSEngine::Synthesize(const std::string& a_text) {
		SSynthesisResult result;
		if (!m_engine || !m_engine->IsInitialized()) {
			result.m_result_code = EResultCode::NotInitialized;
			return result;
		}
		
		std::vector<int16_t> audio = m_engine->Synthesize(a_text.c_str());
		if (audio.empty()) {
			result.m_result_code = EResultCode::ProcessingFailed;
			return result;
		}

		result.m_audio_buffer = CVBTTSAudioBuffer(std::move(audio), m_engine->GetSampleRate());
		result.m_result_code = EResultCode::Success;
		return result;
	}

	CVBTTSAudioBuffer CVBTTSEngine::SynthesizeSimple(const std::string& a_text) {
		if (!m_engine || !m_engine->IsInitialized()) {
			return CVBTTSAudioBuffer();
		}
		std::vector<int16_t> audio = m_engine->Synthesize(a_text.c_str());
		return CVBTTSAudioBuffer(std::move(audio), m_engine->GetSampleRate());
	}

	bool CVBTTSEngine::SynthesizeToWAVFile(const std::string& a_text, const std::string& a_wav_path) {
		if (!m_engine || !m_engine->IsInitialized()) {
			return false;
		}
		try {
			m_engine->SynthesizeToWAVFile(a_text.c_str(), a_wav_path.c_str());
		}
		catch(...) {
			return false;
		}

		return true;

	}

	void CVBTTSEngine::SetSpeakerID(int a_speaker_id) {
		if (m_engine) {
			m_engine->SetSpeakerID(a_speaker_id);
		}
	}

	void CVBTTSEngine::SetSpeed(float a_length_scale) {
		if (m_engine) {
			SSynthesisConfig config = m_engine->GetSynthesisConfig();
			config.m_length_scale = a_length_scale;
			m_engine->SetSynthesisConfig(config);
		}
	}

	void CVBTTSEngine::SetNoiseScale(float a_noise_scale) {
		if (m_engine) {
			SSynthesisConfig config = m_engine->GetSynthesisConfig();
			config.m_noise_scale = a_noise_scale;
			m_engine->SetSynthesisConfig(config);
		}
	}

	void CVBTTSEngine::SetProgressCallback(ProgressCallbackFn a_callback_function) {
		(void)a_callback_function; // TODO: Implement core engine function
	}
	
	void CVBTTSEngine::Cancel() {
		// TODO
	}

	// Factory
	std::unique_ptr<CVBTTSEngine> CVBTTSEngine::Create(const STTSConfig& a_config) {
		try {
			return std::make_unique<CVBTTSEngine>(a_config);
		}
		catch (...) {
			return nullptr;
		}
	}
	
	const char* CVBTTSEngine::GetVersion() { 
		return VB_TTS_VERSION;
	}
	
	STTSConfig CVBTTSEngine::GetDefaultConfig() {
		STTSConfig config;

		config.m_voice_config.m_tashkeel_model_path				= std::nullopt;
		config.m_voice_config.m_model_onnx_path					= "";
		config.m_voice_config.m_model_onnx_json_path			= "";
		config.m_voice_config.m_espeak_data_path				= "";
		config.m_voice_config.m_use_espeak						= true;
		config.m_voice_config.m_use_tashkeel					= false;

		config.m_synthesis_config.m_noise_scale					= 0.667f;
		config.m_synthesis_config.m_length_scale				= 1.0f;
		config.m_synthesis_config.m_noise_width					= 0.8f;
		config.m_synthesis_config.m_sentence_silence_seconds	= 0.2f;
		
		config.m_hardware_config.m_gpu_acceleration_type		= EGPUBackendType::Default;

		return config;
	}

}
#endif