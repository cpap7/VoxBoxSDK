#include "vbppch.h"
#include "TTSAudioStreamBuffer.h"

namespace VoxBox {

	void CCoreTTSAudioStreamBuffer::Push(const std::vector<int16_t>& a_samples) {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_shared_buffer.insert(m_shared_buffer.end(), a_samples.begin(), a_samples.end());
		m_audio_is_ready = true;

		m_condition.notify_one();
	}

	void CCoreTTSAudioStreamBuffer::MarkAsFinished() {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_audio_is_finished = true;
		m_audio_is_ready = true;

		m_condition.notify_one();
	}


	bool CCoreTTSAudioStreamBuffer::WaitAndConsume(std::vector<int16_t>& a_out_samples) {
		std::unique_lock<std::mutex> lock(m_mutex);

		m_condition.wait(lock, [this] {
			return m_audio_is_ready;
		});

		if (m_shared_buffer.empty() && m_audio_is_finished) {
			return false; // No more data
		}

		a_out_samples.insert(a_out_samples.end(), m_shared_buffer.begin(), m_shared_buffer.end());
		m_shared_buffer.clear();


		if (!m_audio_is_finished) {
			m_audio_is_ready = false;
		}

		return true;
	}

	void CCoreTTSAudioStreamBuffer::Reset() {
		std::lock_guard<std::mutex> lock(m_mutex);

		m_shared_buffer.clear();
		//m_internal_buffer.clear();

		m_audio_is_ready = false;
		m_audio_is_finished = false;

	}

	
}