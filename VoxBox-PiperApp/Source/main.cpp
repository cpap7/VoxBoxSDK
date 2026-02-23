#include <iostream>
#include "VoxBoxTTS.h"

int main() {
	const char* model_onnx_path = "Assets/Models/en_US-john-medium.onnx";
	const char* model_onnx_json_path = "Assets/Models/en_US-john-medium.onnx.json";
	const char* test_phrase = "Hello, I am an AI model.";


	// Initialize
	VB_TTS_Init(model_onnx_path, model_onnx_json_path);
	
	// Synthesize to a wav file
	VB_TTS_ToWAVFile(test_phrase, "output.wav");
	
	// Synthesize to raw samples
	int sample_count = 0;
	int16_t* samples = VB_TTS_ToRaw(test_phrase, &sample_count);
	if (samples) {
		std::cout << "Generated " << sample_count << " samples\n";
		VB_TTS_FreeRaw(samples);
	}

	// Cleanup
	VB_TTS_Shutdown();

	return 0;
}