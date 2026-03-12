#pragma once
#include <cstdint>
#include <string>
#include <thread>

namespace VoxBox {
	// NOTE: Most of the magic numbers used are borrowed from the cli.cpp example app from whisper.cpp

	/* File configs */

	// Model config
	struct SSTTModelConfig {
		std::string m_model_path		= "";
		std::string m_initial_prompt	= "";
		std::string m_language			= "en";

		// In-memory model loading
		size_t m_model_data_length		= 0;
		void* m_model_data				= nullptr;

		// Flags
		bool m_translate_to_english		= false;
	};

	// DTW (Dynamic Time Warping) for token timestamps
	struct SDTWConfig {
		std::string m_model_preset = "";        // e.g., "base.en", "large.v3"
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

	/* Hardware configs */

	struct SSTTHardwareConfig {
		int m_thread_count				= std::min(4, (int32_t)std::thread::hardware_concurrency());
		bool m_use_gpu					= true;	
		//bool m_flash_attention		= false;
	};

	// Used for acceleration backend detection at runtime via whisper methods
	struct SSystemConfig {
		std::string m_system_info; // Via whisper_print_info()

		/* Flags */
		// CPU
		bool m_has_noavx	= false;
		bool m_has_avx		= false;
		bool m_has_avx2		= false;
		bool m_has_avx512	= false;

		// GPU
		bool m_has_vulkan	= false;
		bool m_has_cuda		= false;
	};

	// OpenVINO acceleration (if compiled with support)
	struct SOpenVINOConfig {
		std::string m_encode_device = "CPU";  // OpenVINO encoder device
	};

	/* Misc configs */

    // Advanced feature flags
    struct SFeatureConfig {
		std::string m_suppress_regex;      // Regex for token suppression
        std::string m_tdrz_speaker_turn = " [SPEAKER_TURN]";

        bool m_diarize				= false;    // Stereo speaker diarization
        bool m_tinydiarize_enable	= false;    // Model-based diarization (tdrz)
        bool m_detect_language		= false;    // Auto-detect language only
        bool m_suppress_blank		= true;     // Suppress blank outputs
        bool m_suppress_nst			= true;     // Suppress non-speech tokens
        bool m_flash_attn			= false;    // Use flash attention
        bool m_debug_mode			= false;    // Enable debug output
    };


	/* Audio configs */

	// Sampling strategy for decoding
	enum class ESamplingStrategy : int8_t {
		None		= -1,
		
		// Supported strategy types
		Greedy		= 0,
		BeamSearch	= 1,

		Default		= Greedy
	};

	// Decoder tuning params
	struct SDecoderConfig {
		int32_t m_best_of				= 5;	// Greedy	   -- # of best candidates
		int32_t m_beam_size				= 5;	// Beam Search -- beam width
		int32_t m_max_text_context		= -1;	// Max # tokens of a given context (-1 = default)
		int32_t m_max_length			= 0;	// Max segment length (in characters)
		int32_t m_audio_context_size	= 0;	// Audio context size (0 = all)

		ESamplingStrategy m_sampling_strategy = ESamplingStrategy::Default;
	};

	// Grammar-constrained decoding
	struct SGrammarConfig {
		std::string m_gbnf_grammar;         // GBNF grammar string or file path
		std::string m_grammar_rule;         // Top-level rule name
		float m_grammar_penalty = 100.0f;   // Penalty for non-grammar tokens
	};

	// Audio processing offsets
	struct SAudioProcessingConfig {
		int32_t m_offset_t_ms	= 0;        // Time offset in milliseconds
		int32_t m_offset_n		= 0;        // Segment index offset
		int32_t m_duration_ms	= 0;        // Duration to process (0 = all)
		int32_t m_progress_step = 5;        // Progress callback step %
	};

	// Threshold parameters for quality control
	struct SThresholdConfig {
		float m_word_thold		= 0.01f;   // Word timestamp probability threshold
		float m_entropy_thold	= 2.40f;   // Entropy threshold for decoder fail
		float m_logprob_thold	= -1.00f;  // Log probability threshold for fail
		float m_no_speech_thold = 0.6f;    // No speech threshold
	};

	// Temperature settings for sampling
	struct STemperatureConfig {
		float m_temperature		= 0.0f;    // Initial sampling temperature
		float m_temperature_inc = 0.2f;    // Temperature increment on fallback
		bool  m_no_fallback		= false;   // Disable temperature fallback
	};

	///////////////////////////////////////////////////////////
	
    // Top-level config struct for interfacing w/ whisper.cpp
	// Used in creating the API class CVBSTTEngine (see VoxBoxSTT.h)
	// and the internal core engine class CCoreSTTEngine (see STTEngine.h)
	struct VB_STT_API SSTTConfig {
		SSTTModelConfig         m_model_config;
        SGrammarConfig          m_grammar_config;
        SFeatureConfig          m_feature_config;
        SDTWConfig              m_dtw_config;
        SSystemConfig           m_system_config;
        SOpenVINOConfig         m_open_vino_config;
        SDecoderConfig          m_decoder_config;
        SAudioProcessingConfig  m_audio_processing_config;
        SThresholdConfig        m_threshold_config;
        STemperatureConfig      m_temperature_config;
		SSTTHardwareConfig      m_hardware_config;
        SOutputConfig           m_output_config;
	};
}