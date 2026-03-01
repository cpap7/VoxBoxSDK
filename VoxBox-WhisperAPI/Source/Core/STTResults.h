#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace VoxBox {

	struct SLanguageResult {
		std::string m_language_code = "";
		float m_confidence = 0.0f;

		EResultCode m_result_code = EResultCode::NotInitialized;

		inline const std::string& LanguageCode() { return m_language_code;	}
		inline float Confidence() const			 { return m_confidence;		}
		inline EResultCode ResultCode() const	 { return m_result_code;	}

		inline bool Success() const { return m_result_code == EResultCode::Success; }
		inline operator bool()		{ return Success(); }
	};

	struct STranscriptResult {
		std::string m_text = "";

		std::vector<int> m_part_indices;
		std::vector<float> m_word_probabilities;

		EResultCode m_result_code = EResultCode::NotInitialized;

		inline const std::string& Text() const		{ return m_text;						}

		inline int* PartIndicesData()				{ return m_part_indices.data();			}
		inline size_t PartIndicesSize() const		{ return m_part_indices.size();			}
		inline float* WordProbabilitiesData()		{ return m_word_probabilities.data();	}
		inline size_t WordProbabilitiesSize() const { return m_word_probabilities.size();	}

		inline EResultCode ResultCode() const		{ return m_result_code;					}

		inline bool Success() const					{ return m_result_code == EResultCode::Success; }
		inline operator bool()						{ return Success(); }
	};

	
}