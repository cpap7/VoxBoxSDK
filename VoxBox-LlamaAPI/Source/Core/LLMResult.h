#pragma once

#include <string>
#include <vector>
#include <cstdint>

//#include "../../VoxBox-Common/Source/VBCommon.h"

namespace VoxBox {
	struct SInferenceResult {
		std::string m_text = "";
		EResultCode m_result_code = EResultCode::NotInitialized;

		inline const std::string& Text() const	{ return m_text;		}
		inline EResultCode ResultCode() const	{ return m_result_code; }
		
		inline bool Success() const { return m_result_code == EResultCode::Success; }
		inline operator bool()		{ return Success();								}
	};

	struct SEmbeddingResult {
		std::vector<float> m_embeddings;
		EResultCode m_result_code = EResultCode::NotInitialized;

		inline const float* EmbeddingData() const	{ return m_embeddings.data();	}
		inline size_t EmbeddingSize() const			{ return m_embeddings.size();	}
		inline EResultCode ResultCode() const		{ return m_result_code;			}

		inline bool Success() const { return m_result_code == EResultCode::Success; }
		inline operator bool()		{ return Success();								}
	};
}