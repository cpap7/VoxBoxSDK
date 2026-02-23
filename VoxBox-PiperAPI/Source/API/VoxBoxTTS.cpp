#include "vbppch.h"
#include "VoxBoxTTS.h"

#include "../Core/TTSEngine.h"
#include "../Configs/PiperConfig.h"

#include <piper.hpp>

// Globals
static std::unique_ptr<VoxBox::CTTSEngine> g_tts_engine = nullptr;

VB_TTS_API void __stdcall VB_TTS_Init(const char* a_model_onnx_path, const char* a_model_json_path) {
	VoxBox::SVBPiperConfig config;
	config.m_voice_config.m_model_path = a_model_onnx_path;
	config.m_voice_config.m_model_config_path = a_model_json_path;
	
	g_tts_engine = std::make_unique<VoxBox::CTTSEngine>(config); // Init called upon construction
}

VB_TTS_API void __stdcall VB_TTS_Reinit(const char* a_model_onnx_path, const char* a_model_json_path) {
	if (!g_tts_engine) {
		VB_TTS_Init(a_model_onnx_path, a_model_json_path);
		return;
	}

	VoxBox::SVBPiperConfig config = g_tts_engine->GetConfig();
	config.m_voice_config.m_model_path = a_model_onnx_path;
	config.m_voice_config.m_model_config_path = a_model_json_path;

	g_tts_engine->Reinit(config);
}

VB_TTS_API void __stdcall VB_TTS_Shutdown() {
	if (g_tts_engine) {
		g_tts_engine->Shutdown();
		g_tts_engine.reset();
	}
}

VB_TTS_API void __stdcall VB_TTS_FreeRaw(const int16_t* a_samples) {
	//assert(a_samples != nullptr);
	if (a_samples) {
		free(const_cast<int16_t*>(a_samples));
	}
}

VB_TTS_API void __stdcall VB_TTS_StreamToStreamRaw(const void* a_input_stream, const void* a_output_stream) {
	assert(g_tts_engine && g_tts_engine->IsInitialized());
	assert(a_input_stream && a_output_stream);

	auto* input = const_cast<std::istream*>(static_cast<const std::istream*>(a_input_stream));
	auto* output = const_cast<std::ostream*>(static_cast<const std::ostream*>(a_output_stream));
	
	g_tts_engine->SynthesizeStreaming(*input, *output);
}

VB_TTS_API int16_t* __stdcall VB_TTS_ToRaw(char const* const a_text, int* const a_out_sample_count) {
	assert(g_tts_engine && g_tts_engine->IsInitialized());
	assert(a_text && a_out_sample_count);
	
	int16_t* result = nullptr;
	std::vector<int16_t> audio_buffer = g_tts_engine->Synthesize(a_text);
	
	if (audio_buffer.empty()) {
		assert(false);
		return nullptr;
	}

	*a_out_sample_count = static_cast<int>(audio_buffer.size());
	size_t byte_count = static_cast<size_t>(*a_out_sample_count) * sizeof(int16_t);
	result = reinterpret_cast<int16_t*>(malloc(byte_count));
	
	if (!result) {
		assert(false);
		return nullptr;
	}

	std::memcpy(result, audio_buffer.data(), byte_count);
	return result;
}

VB_TTS_API void __stdcall VB_TTS_ToWAVFile(const char* a_text, const char* a_wav_path) {
	assert(g_tts_engine && g_tts_engine->IsInitialized());
	assert(a_text && a_wav_path);

	g_tts_engine->SynthesizeToWAVFile(a_text, a_wav_path);
}
