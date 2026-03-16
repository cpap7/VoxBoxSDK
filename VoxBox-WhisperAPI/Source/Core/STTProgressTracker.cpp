#include "vbwpch.h"
#include "STTProgressTracker.h"

namespace VoxBox {

	void CSTTProgressTracker::ReportProgress(int a_part_progress) {
		if (!m_callback_function) {
			return;
		}

		int full_progress = a_part_progress;
		if (m_parts_count > 1) {
			full_progress = (100 * m_current_part + a_part_progress) / m_parts_count; // For example, index = 3, count = 5 --> (100 * 3 + progress) / 5 
		}
		
		// Only report if progress changed by at least m_progress_step
		if (full_progress - m_last_reported_progress < m_progress_step) {
			return;
		}

		m_last_reported_progress = full_progress;
		m_callback_function(full_progress);
	}
	
	void CSTTProgressTracker::Reset() {
		m_is_aborted.store(false);
		m_parts_count = 1;
	}
}