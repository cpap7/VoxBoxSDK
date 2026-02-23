#include "vbwpch.h"
#include "STTEngine.h"

namespace VoxBox {
	CSTTEngine::CSTTEngine(const SVBWhisperConfig& a_config) 
		: m_config(a_config) {

		Init(a_config);
	}

	CSTTEngine::~CSTTEngine() {
		Shutdown();
	}

	bool CSTTEngine::Init(const SVBWhisperConfig& a_config) {

	}

	void CSTTEngine::Shutdown() {

	}

	STranscriptResult CSTTEngine::Transcribe(const float* a_audio_data, int a_audio_length, bool a_get_word_probabilities = false) {

	}

	STranscriptResult CSTTEngine::TranscribeParts(const float* a_audio_data, const std::vector<int>& a_part_starts, const std::vector<int>& a_part_ends, bool a_get_word_probabilities = false) {

	}

	void CSTTEngine::SetProgressCallback(ProgressCallbackFn a_callback) {

	}
	
	void CSTTEngine::Cancel() {

	}

	SSystemConfig CSTTEngine::GetSystemConfig() {

	}

	std::string CSTTEngine::ExtractText(bool a_get_word_probabilities, std::vector<float>& a_probabilities) {

	}
}