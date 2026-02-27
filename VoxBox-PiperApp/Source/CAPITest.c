#include <stdio.h>
#include <stdlib.h>

#include <VoxBoxTTS.h>

// NOTE:
// This goes without saying, but don't include both test files 
// while compiling, since they both define main()

int main() {
	const char* model_onnx_path = "Assets/Models/en_US-john-medium.onnx";
	const char* model_onnx_json_path = "Assets/Models/en_US-john-medium.onnx.json";

	// Get default config & set paths
	VB_TTS_Config_t config = VB_TTS_GetDefaultConfig();
	config.m_model_onnx_path = model_onnx_path;
	config.m_model_onnx_json_path = model_onnx_json_path;

	printf("[VoxBox] TTS Version: %s\n", VB_TTS_GetVersion());

	// Create engine & validate
	VB_TTS_EngineHandle_t engine = VB_TTS_Create(&config);
	if (!engine) {
		printf("[VoxBox] Failed to create TTS engine!\n");
		return 1;
	}
	
	if (!VB_TTS_IsLoaded(engine)) {
		printf("[VoxBox] TTS engine not loaded!\n");
		VB_TTS_Destroy(engine);
		return 1;
	}
	printf("[VoxBox] TTS engine created and loaded successfully!\n");

	// Strings for testing
	const char* test_phrase = "Hello, I am an AI model.";
	const char* wav_file_path = "output_c.wav";

	{ // Synthesize to audio result struct

		int sample_count = 0;
		int16_t* samples = VB_TTS_SynthesizeSimple(engine, test_phrase, &sample_count);

		if (samples && sample_count > 0) {
			printf("[VoxBox] Simple synthesis result: %d samples\n", sample_count);
			VB_TTS_FreeSamples(samples);
		}
		else {
			printf("[VoxBox] Simple synthesis failed!");
		}
	}

	{ // Synthesize to WAV file
		if (VB_TTS_SynthesizeToWAVFile(engine, test_phrase, wav_file_path)) {
			printf("[VoxBox] Successfully saved WAV file: %s\n", wav_file_path);
		}
		else {
			printf("[VoxBox] Failed to save WAV file!\n");
		}
	}

	{ // Adjust settings
		VB_TTS_SetSpeed(engine, 0.9f); // Slightly faster
		VB_TTS_SetSpeaker(engine, 0);
	}

	// Cleanup
	VB_TTS_Destroy(engine);
	printf("[VoxBox] Test complete!\n");
	
	return 0;
}