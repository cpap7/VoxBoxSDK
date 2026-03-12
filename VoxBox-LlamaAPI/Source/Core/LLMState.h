#pragma once
#include <cstdint>
#include <vector>
#include <string>

namespace VoxBox {
	// Serialized LLM context state snapshot
	// Holds raw llama.cpp context bytes + metadata
	// Needed for resuming inference from an exact point of capture (or snapshot)
	struct SLLMSnapshot {
		std::vector<uint8_t> m_data;
		int m_token_count		= -1;
		int m_last_token_type	= 0;
		
		inline bool IsEmpty() const { return m_data.empty();	}
		inline size_t Size() const	{ return m_data.size();		}
	};
}