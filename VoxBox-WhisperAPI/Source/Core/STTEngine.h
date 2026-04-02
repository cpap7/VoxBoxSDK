#pragma once
#include "STTConfig.h"
#include "STTProgressTracker.h"
#include "STTResults.h"

#include <vector>
#include <memory>

// Forward declarations
struct whisper_context;
struct whisper_full_params;

namespace VoxBox {
	
	class CSTTEngineImpl {
	private:
		SSTTConfig m_config;
		CSTTProgressTracker m_progress_tracker;

		// whisper.cpp 
		whisper_context* m_context = nullptr;

	public:
		explicit CSTTEngineImpl(const SSTTConfig& a_config);
		~CSTTEngineImpl();

		void Init();
		void Shutdown();

		STranscriptResult Transcribe(const float* a_audio_data, int a_audio_length, bool a_get_word_probabilities = true);
		STranscriptResult TranscribeParts(const float* a_audio_data, const std::vector<int>& a_part_starts, 
			const std::vector<int>& a_part_ends, bool a_get_word_probabilities = true);

		SLanguageResult DetectLanguage(const float* a_audio_data, int a_audio_length);

		// Setters
		inline void SetProgressCallback(ProgressCallbackFn a_callback)	{ m_progress_tracker.SetCallback(a_callback);	}
		inline void Cancel()											{ m_progress_tracker.Cancel();					}

		// Getters
		inline bool IsLoaded() const									{ return m_context != nullptr;		}
		inline const SSystemConfig& GetSystemConfig() const				{ return m_config.m_system_config;	}
		

	private: // Helpers
		std::string ExtractTextTokens(std::vector<float>& a_probabilities, bool a_get_word_probabilities);
		void UpdateWhisperContext(whisper_full_params& a_params, int a_part_index);
		void UpdateWhisperParams(whisper_full_params& a_params, int a_part_index = 0);
	};
}

