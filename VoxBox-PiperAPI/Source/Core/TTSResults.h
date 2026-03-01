#pragma once
#include <vector>
#include <cstdint>

namespace VoxBox {
	struct SSynthesisResult {
		std::vector<int16_t> m_samples;
		int m_sample_rate = 22050; // Piper default
		EResultCode m_result_code = EResultCode::NotInitialized;

		inline const int16_t* SampleData() const	 { return m_samples.data();						}
		inline int SampleCount() const				 { return static_cast<int>(m_samples.size());	}
		inline int SampleRate() const				 { return m_sample_rate;						}
		inline EResultCode ResultCode() const		 { return m_result_code;						}
		inline bool IsEmpty() const					 { return m_samples.empty();					}

		inline bool Success() const { return m_result_code == EResultCode::Success; }
		inline operator bool() { return Success(); }

		inline void MoveSamples(std::vector<int16_t>&& a_samples) { m_samples = std::move(a_samples); }


		
	};
}