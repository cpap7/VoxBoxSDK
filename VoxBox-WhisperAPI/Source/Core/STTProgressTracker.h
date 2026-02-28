#pragma once
#include <functional>
#include <atomic>

namespace VoxBox {
	using ProgressCallbackFn = std::function<void(int)>;
	
	class CCoreSTTProgressTracker {
	private:
		ProgressCallbackFn m_callback_function; // Defined by the user

		int m_parts_count				= 1;
		int m_current_part				= 0;
		int m_progress_step				= 5;
		int m_last_reported_progress	= 0;

		std::atomic<bool> m_is_aborted{ false };
		
	public:
		void ReportProgress(int a_part_progress);
		void Reset();
		
		// Getters
		inline bool IsAborted() const									{ return m_is_aborted.load();									 }

		// Setters
		inline void Cancel()											{ m_is_aborted.store(true);										 }
		inline void SetCallback(ProgressCallbackFn a_callback_function) { m_callback_function = a_callback_function;					 }
		inline void SetPartsCount(int a_parts_count)					{ m_parts_count = (a_parts_count > 0) ? a_parts_count : 1;		 }
		inline void SetCurrentPart(int a_current_part)					{ m_current_part = a_current_part;								 }
		inline void SetProgressStep(int a_progress_step)				{ m_progress_step = (a_progress_step > 0) ? a_progress_step : 5; }
	};
}

