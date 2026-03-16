#include <iostream>
#include <vector>

#include <VoxBoxTTS.h>
#include <VoxBoxSTT.h>

/* API test app for both STT & TTS */

std::vector<float> Resample(const std::vector<float>& a_input, double a_source_rate, double a_target_rate) {
	double ratio = a_source_rate / a_target_rate; // Piper(22kHz) / Whisper(16kHz) = ~1.378 

	std::vector<float> output_samples;
	size_t new_size = static_cast<size_t>(a_input.size() / ratio);
	output_samples.reserve(new_size);

	for (size_t i{}; i < new_size; ++i) {
		double src_idx = i * ratio;
		int index = static_cast<int>(src_idx);
		
		double frac = src_idx - index;

		// Linear interpolation
		if (index + 1 < a_input.size()) {
			// (1-f) * a + f * b
			float sample = (1.0f - frac) * a_input[index] + frac * a_input[index + 1];
			output_samples.push_back(sample);
		}
		else {
			output_samples.push_back(a_input[index]);
		}
	}

	return output_samples;
}

std::vector<float> ConvertInt16ToFloatVector(const std::vector<int16_t>& a_input) {
	std::vector<float> output_vector;
	output_vector.reserve(a_input.size());

	// Normalize
	constexpr float scale = 1.0f / 32768.0f;

	for (int16_t value : a_input) {
		output_vector.push_back(static_cast<float>(value) * scale);
	}

	return output_vector;
}

int main() {
	// Text-to-speech engine setup
	const char* tts_model_onnx_path = "Assets/Models/en_US-john-medium.onnx";
	const char* tts_model_onnx_json_path = "Assets/Models/en_US-john-medium.onnx.json";
	
	auto tts_config = VoxBox::CTTSEngine::GetDefaultConfig();
	tts_config.m_voice_config.m_model_onnx_path = tts_model_onnx_path;
	tts_config.m_voice_config.m_model_onnx_json_path = tts_model_onnx_json_path;

	auto tts_engine = VoxBox::CTTSEngine::Create(tts_config);
	if (!tts_engine || !tts_engine->IsLoaded()) {
		std::cerr << "[VoxBox] Failed to load TTS engine\n";
		return 1;
	}

	// Speech-to-text engine setup
	const char* stt_initial_prompt = "Transcribe the audio samples";
	const char* stt_model_path = "Assets/Models/ggml-tiny.bin";
	const char* stt_language = "en";

	VoxBox::SSTTConfig config = VoxBox::CSTTEngine::GetDefaultConfig();
	config.m_model_config.m_initial_prompt = stt_initial_prompt;
	config.m_model_config.m_model_path = stt_model_path;
	config.m_model_config.m_language = stt_language;

	auto stt_engine = VoxBox::CSTTEngine::Create(config);
	if (!stt_engine || !stt_engine->IsLoaded()) {
		std::cerr << "[VoxBox] Failed to load STT engine\n";
		return 1;
	}

	// Convert text to audio via TTS engine
	const std::string text_to_convert = "Hello. I am an AI model and I am going to steal your job";
	VoxBox::SAudioResult tts_result = tts_engine->Synthesize(text_to_convert);
	
	if (!tts_result.Success()) {
		std::cerr << "[VoxBox] TTS synthesis failed\n";
		return 1;
	}
	std::cout << "[VoxBox] TTS output: " << tts_result.SampleCount() << " samples @ " << tts_result.m_sample_rate << " Hz\n";

	
	// Resample audio
	//constexpr double piper_sample_rate = 22050.0;	// Output audio samples will be @ 22kHz
	constexpr double whisper_sample_rate = 16000.0f;	// Only takes in audio samples @ 16kHz
	auto samples_float = ConvertInt16ToFloatVector(tts_result.m_samples);
	std::vector<float> stt_input = Resample(samples_float, static_cast<double>(tts_result.m_sample_rate), whisper_sample_rate);

	// Transcribe via STT
	VoxBox::STranscriptResult stt_transcript_result = stt_engine->Transcribe(stt_input.data(), static_cast<int>(stt_input.size()));

	std::cout << "[VoxBox] STT transcript result:\n"
		<< "- Part indices: " << stt_transcript_result.m_part_indices.size() << "\n"
		<< "- Text: " << stt_transcript_result.m_text << "\n"
		<< "- Word probabilities: " << stt_transcript_result.m_word_probabilities.size() << "\n";


	if (stt_transcript_result.Success()) {
		std::cout << "[VoxBox] STT transcription: " << stt_transcript_result.m_text << "\n";
	}
	else {
		std::cerr << "[VoxBox] STT transcription failed\n";
		return 1;
	}

	return 0;
}