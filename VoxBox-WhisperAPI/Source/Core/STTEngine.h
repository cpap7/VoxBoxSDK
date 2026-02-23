#pragma once
#include "./Configs/WhisperConfig.h"
#include "./Utilities/ProgressTracker.h"

#include <vector>
#include <string>
#include <memory>

// Forward declarations
struct whisper_context;

namespace VoxBox {
	
	struct VB_STT_API STranscriptResult {
		std::string m_text;
		
		std::vector<float> m_word_probabilities;
		std::vector<int> m_part_indices;

		bool m_sucess = false;
	};
	

	class VB_STT_API CSTTEngine {
	private:
		SVBWhisperConfig m_config;
		CProgressTracker m_progress_tracker;
		whisper_context* m_context = nullptr;

	public:
		CSTTEngine();
		~CSTTEngine();

		bool Init(const SVBWhisperConfig& a_config);
		void Shutdown();

		STranscriptResult Transcribe(const float* a_audio_data, int a_audio_length, bool a_get_word_probabilities = false);
		STranscriptResult TranscribeParts(const float* a_audio_data, const std::vector<int>& a_part_starts, const std::vector<int>& a_part_ends, bool a_get_word_probabilities = false);

		void SetProgressCallback(ProgressCallbackFn a_callback);
		void Cancel();

		SSystemConfig GetSystemConfig();


	private:
		std::string ExtractText(bool a_get_word_probabilities, std::vector<float>& a_probabilities);

	};
}

