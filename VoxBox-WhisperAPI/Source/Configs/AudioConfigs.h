#pragma once
#include <cstdint>
#include <string>

namespace VoxBox {
	// Sampling strategy for decoding
	enum class VB_STT_API ESamplingStrategy : uint8_t {
		None		= 0,

		// Supported strategies
		Greedy		= 1,
		BeamSearch	= 2,

		Default		= Greedy
	};

	// Decoder tuning params
	struct VB_STT_API SDecoderConfig {
		int32_t m_best_of				= 5;	// Greedy	   -- # of best candidates
		int32_t m_beam_size				= 5;	// Beam Search -- beam width
		int32_t m_max_context			= -1;	// Max # tokens of a given context (-1 = default)
		int32_t m_max_length			= 0;	// Max segment length (in characters)
		int32_t m_audio_context_size	= 0;	// Audio context size (0 = all)

		ESamplingStrategy m_sampling_strategy = ESamplingStrategy::Default;
	};

	// Grammar-constrained decoding
	struct VB_STT_API SGrammarConfig {
		std::string m_gbnf_grammar;         // GBNF grammar string or file path
		std::string m_grammar_rule;         // Top-level rule name
		float m_grammar_penalty = 100.0f;   // Penalty for non-grammar tokens
	};

	// Audio processing offsets
	struct VB_STT_API SAudioProcessingConfig {
		int32_t m_offset_t_ms	= 0;        // Time offset in milliseconds
		int32_t m_offset_n		= 0;        // Segment index offset
		int32_t m_duration_ms	= 0;        // Duration to process (0 = all)
		int32_t m_progress_step = 5;        // Progress callback step %
	};

	// Threshold parameters for quality control
	struct VB_STT_API SThresholdConfig {
		float m_word_thold		= 0.01f;   // Word timestamp probability threshold
		float m_entropy_thold	= 2.40f;   // Entropy threshold for decoder fail
		float m_logprob_thold	= -1.00f;  // Log probability threshold for fail
		float m_no_speech_thold = 0.6f;    // No speech threshold
	};

	// Temperature settings for sampling
	struct VB_STT_API STemperatureConfig {
		float m_temperature		= 0.0f;    // Initial sampling temperature
		float m_temperature_inc = 0.2f;    // Temperature increment on fallback
		bool  m_no_fallback		= false;   // Disable temperature fallback
	};
}