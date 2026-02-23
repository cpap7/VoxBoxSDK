#pragma once
#include <functional>
#include <atomic>

namespace VoxBox {
	using ProgressCallbackFn = std::function<void(int)>;
	
	
	class VB_STT_API CProgressTracker {
	private:
		ProgressCallbackFn m_callback_function;
		int m_parts_count = 1;

		std::atomic<bool> m_aborted{ false };
		
	public:
		void SetCallback(ProgressCallbackFn a_callback);
		void SetPartCount(int a_count);
		void ReportProgress(int a_part_index, int a_part_progress);
		void Cancel();
		bool IsAborted() const;
		void Reset();
	};
}

