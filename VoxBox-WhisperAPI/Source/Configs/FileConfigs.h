#pragma once
#include <string>

namespace VoxBox {

	// Model config
	struct SModelConfig {
		std::string m_model_path	 = "";
		std::string m_initial_prompt = "";
		std::string m_language		 = "en";

		// In-memory model loading
		size_t m_model_data_length	= 0;
		void* m_model_data			= nullptr;

		// Flags
		bool m_translate_to_english = false;
	};

	// DTW (Dynamic Time Warping) for token timestamps
	struct SDTWConfig {
		std::string m_model_preset;        // e.g., "base.en", "large.v3"
		bool m_enabled = false;
	};

	// Output format flags
	struct SOutputConfig {
		bool m_timestamps		= true;     // Include timestamps
		bool m_word_timestamps	= false;    // Token-level timestamps
		bool m_split_on_word	= false;    // Split on word vs token
		bool m_print_special	= false;    // Print special tokens
		bool m_print_colors		= false;    // Colored output by confidence
		bool m_print_progress	= false;    // Print progress updates
		bool m_log_score		= false;    // Log decoder scores
	};
}