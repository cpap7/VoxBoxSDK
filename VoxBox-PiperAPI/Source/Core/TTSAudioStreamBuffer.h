#pragma once
#include <mutex>
#include <condition_variable>
#include <vector>
#include <cstdint>

namespace VoxBox {
	class CCoreTTSAudioStreamBuffer {
	private:
		std::mutex m_mutex;
		std::condition_variable m_condition;		// For consumer thread: blocking 

		std::vector<int16_t> m_shared_buffer;		// For producer thread: writes samples here under lock
		//std::vector<int16_t> m_internal_buffer;

		int m_sample_rate = 22050; // Piper default

		// Handoff flags (between threads)
		bool m_audio_is_ready = false;
		bool m_audio_is_finished = false;

	public:
		CCoreTTSAudioStreamBuffer() = default;
		~CCoreTTSAudioStreamBuffer() = default;

		// Producer functions
		// Called by synthesis thread
		void Push(const std::vector<int16_t>& a_samples);
		void MarkAsFinished();

		// Consumer functions
		// Called by output thread
		bool WaitAndConsume(std::vector<int16_t>& a_out_samples);

		// Utility functions
		void Reset();

		// Setters
		inline void SetSampleRate(int a_rate) { m_sample_rate = a_rate; }

		// Getters
		inline int SampleCount() const		{ return static_cast<int>(m_shared_buffer.size());	}
		inline int SampleRate() const		{ return m_sample_rate;								}
		inline const int16_t* Data() const	{ return m_shared_buffer.data();					}
		inline bool IsEmpty() const			{ return m_shared_buffer.empty();					}
	};
}

