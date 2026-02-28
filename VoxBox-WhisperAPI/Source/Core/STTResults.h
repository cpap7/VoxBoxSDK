#pragma once
#include <string>
#include <vector>
#include <cstdint>

namespace VoxBox {
	struct SLanguageResult {
		std::string m_language_code;
		float m_confidence = 0.0f;
	};

	struct STranscriptResult {
		std::string m_text;

		std::vector<int> m_part_indices;
		std::vector<float> m_word_probabilities;

		bool m_success = false;
	};
}