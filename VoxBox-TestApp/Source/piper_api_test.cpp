#include <iostream>
#include <cstdlib>

#include <VoxBoxTTS.h>

// NOTE:
// This goes without saying, but don't include both test files 
// while compiling, since they both define main()

int main() {
	// Model paths for testing purposes
	const char* model_onnx_path = "Assets/Models/en_US-john-medium.onnx";
	const char* model_onnx_json_path = "Assets/Models/en_US-john-medium.onnx.json";

	// Setup config
	auto config = VoxBox::CTTSEngine::GetDefaultConfig();
	config.m_voice_config.m_model_onnx_path = model_onnx_path;
	config.m_voice_config.m_model_onnx_json_path = model_onnx_json_path;

	// Setup engine & validate
	auto engine = VoxBox::CTTSEngine::Create(config);
	if (!engine) {
		std::cerr << "[VoxBox] Failed to create the TTS engine!\n";
		return 1;
	}
	if (!engine->IsLoaded()) {
		std::cerr << "[VoxBox] Failed to load the TTS engine!\n";
		return 1;
	}
	std::cout << "[VoxBox] TTS version: " << engine->GetVersion() << "\n";
	std::cout << "[VoxBox] TTS engine created and loaded successfully!\n";

	// Strings for testing
	std::string test_phrase = "Hello. I am an AI model and I am going to steal your job";
	std::string wav_file_path = "output_cpp.wav";

	{ // Test audio synthesis result
		VoxBox::SAudioResult result = engine->Synthesize(test_phrase);
		if (result.Success()) {
			std::cout << "[VoxBox] Audio synthesis succeeded: " 
				<< result.SampleCount() << " samples @ " 
				<< result.SampleRate() << " Hz\n";
		}
		else {
			std::cerr << "[VoxBox] Audio synthesis failed!\n";
		}
	
	}

	{ // Test audio synthesis to WAV file
		if (engine->SynthesizeToWAVFile(test_phrase, wav_file_path)) {
			std::cout << "[VoxBox] WAV file created at: " << wav_file_path << "\n"
				<< "For phrase:" << "'" << test_phrase << "'\n";
		}
		else {
			std::cerr << "[VoxBox] Failed to generate & save WAV file!\n";
		}
	}

	engine->SetSpeed(1.2f);			// Slower
	engine->SetNoiseScale(0.5f);	// Less noise variation
	engine->SetSpeakerID(0);

	std::cout << "[VoxBox] Test complete!\n";

	return 0;
}