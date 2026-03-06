#include "vbwpch.h"
#include "STTEngine.h"
#include "STTResults.h"

#include <whisper.h>

namespace VoxBox {
	
	namespace Utilities {
		static void WhisperProgressCallback(whisper_context* a_context, whisper_state* a_state, int a_progress, void* a_user_data) {
			auto* tracker = static_cast<CCoreSTTProgressTracker*>(a_user_data);
			if (tracker) {
				tracker->ReportProgress(a_progress); // Current part would be tracked internally
			}
		}

		static bool WhisperAbortCallback(void* a_user_data) {
			auto* tracker = static_cast<CCoreSTTProgressTracker*>(a_user_data);
			return tracker && tracker->IsAborted();
		}

		static bool WhisperEncoderBeginCallback(whisper_context* a_context, whisper_state* a_state, void* a_user_data) {
			return !WhisperAbortCallback(a_user_data);
		}

		static ESamplingStrategy WhisperSamplingStrategyToVoxBox(whisper_sampling_strategy a_strategy) {
			switch (a_strategy) {
				case WHISPER_SAMPLING_GREEDY:		return ESamplingStrategy::Greedy;
				case WHISPER_SAMPLING_BEAM_SEARCH:	return ESamplingStrategy::BeamSearch;
			}

			assert(false && "[VoxBox] Unknown sampling strategy!");
			return ESamplingStrategy::Greedy; // Return default
		}

		static whisper_sampling_strategy VoxBoxSamplingStrategyToWhisper(ESamplingStrategy a_strategy) {
			switch (a_strategy) {
				case ESamplingStrategy::None:		break;
				case ESamplingStrategy::Greedy:		return WHISPER_SAMPLING_GREEDY;
				case ESamplingStrategy::BeamSearch: return WHISPER_SAMPLING_BEAM_SEARCH;
			}

			assert(false && "[VoxBox] Unknown sampling strategy!");
			return WHISPER_SAMPLING_GREEDY; // Return default
		}
	}


	CSTTEngineImpl::CSTTEngineImpl(const SSTTConfig& a_config)
		: m_config(a_config) {
		Init();
	}

	CSTTEngineImpl::~CSTTEngineImpl() {
		Shutdown();
	}

	void CSTTEngineImpl::Init() {

		whisper_context_params context_params = whisper_context_default_params();
		context_params.use_gpu = m_config.m_hardware_config.m_use_gpu;
		context_params.flash_attn = m_config.m_feature_config.m_flash_attn;

		m_context = whisper_init_from_file_with_params(m_config.m_model_config.m_model_path.c_str(), context_params);

		if (!m_context && m_config.m_hardware_config.m_use_gpu) {
			// Fall back to CPU if GPU init fails
			context_params.use_gpu = false;
			m_context = whisper_init_from_file_with_params(m_config.m_model_config.m_model_path.c_str(), context_params);
		}
	}

	void CSTTEngineImpl::Shutdown() {
		if (m_context) {
			whisper_free(m_context);
			m_context = nullptr;
		}
	}

	STranscriptResult CSTTEngineImpl::Transcribe(const float* a_audio_data, int a_audio_length, bool a_get_word_probabilities) {
		STranscriptResult result;

		if (!m_context) {
			return result;
		}

		// Reset tracker so we can have a new transcription
		m_progress_tracker.Reset();
		m_progress_tracker.SetPartsCount(1);
		m_progress_tracker.SetCurrentPart(0);

		// Configure whisper parameters
		whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
		UpdateWhisperParams(params, 0);

		// Run transcription
		if (whisper_full(m_context, params, a_audio_data, a_audio_length) != 0) {
			return result;
		}

		result.m_text			= ExtractTextTokens(result.m_word_probabilities, a_get_word_probabilities);
		result.m_result_code	= EResultCode::Success;

		return result;
	}

	STranscriptResult CSTTEngineImpl::TranscribeParts(const float* a_audio_data, const std::vector<int>& a_part_starts,
		const std::vector<int>& a_part_ends, bool a_get_word_probabilities) {

		STranscriptResult result;

		if (!m_context || a_part_starts.size() != a_part_ends.size() || a_part_starts.empty()) {
			return result;
		}

		int parts_count = static_cast<int>(a_part_starts.size());

		// Reset tracker so we can have a new transcription
		m_progress_tracker.Reset();
		m_progress_tracker.SetPartsCount(parts_count);

		std::string full_text = "";
		static constexpr int min_audio_samples = 16000; // ~1 sec minimum @ 16kHz

		// Configure whisper parameters
		whisper_full_params params = whisper_full_default_params(WHISPER_SAMPLING_GREEDY);
		UpdateWhisperParams(params, 0);

		// Populate the text w/ transcription
		for (int i = 0; i < parts_count; ++i) {
			// Break out early if cancelled
			if (m_progress_tracker.IsAborted()) {
				break; 
			}

			m_progress_tracker.SetCurrentPart(i);

			// Calculate part bounds
			int part_start = a_part_starts[i];
			int part_length = a_part_ends[i] - part_start;
			const float* part_data = a_audio_data + part_start;

			// Pad short audio (~1 second minimum)
			std::vector<float> padded_buffer;
			if (part_length < min_audio_samples) {
				padded_buffer.resize(min_audio_samples, 0.0f);
				std::copy(part_data, part_data + part_length, padded_buffer.begin());
				
				part_data = padded_buffer.data();
				part_length = min_audio_samples;
			}

			// Update context
			// Keep context between parts = false
			// Reset on first part only = true
			params.no_context = (i == 0) ? true : false;
			//UpdateWhisperContext(params, i);

			// Run transcription
			if (whisper_full(m_context, params, part_data, part_length) != 0) {
				return result; // Transcription error
			}

			// Extract this part's text
			std::string part_text = ExtractTextTokens(result.m_word_probabilities, a_get_word_probabilities);
			if (!part_text.empty()) {
				result.m_part_indices.push_back(static_cast<int>(full_text.length()));
				full_text += part_text;
			}
			else {
				result.m_part_indices.push_back(-1); // Mark empty parts
			}
		}

		result.m_text			= full_text;
		result.m_result_code	= (!m_progress_tracker.IsAborted()) ? EResultCode::Success : EResultCode::Cancelled;

		return result;
	}

	SLanguageResult CSTTEngineImpl::DetectLanguage(const float* a_audio_data, int a_audio_length) {
		SLanguageResult result;
		if (!m_context || !a_audio_data || a_audio_length <= 0) {
			return result;
		}

		// whisper_lang_auto_detect needs, at minimum, 1 second of audio (16000 samples @ 16kHz)
		static constexpr int min_samples = 16000;
		std::vector<float> padded_buffer;

		const float* audio_ptr = a_audio_data;
		int audio_len = a_audio_length;

		if (a_audio_length < min_samples) {
			padded_buffer.resize(min_samples, 0.0f);
			std::copy(a_audio_data, a_audio_data + a_audio_length, padded_buffer.begin());
			audio_ptr = padded_buffer.data();
			audio_len = min_samples;
		}
		
		int lang_count = whisper_lang_max_id() + 1;
		std::vector<float> lang_probs(lang_count, 0.0f);
		
		// The language ID is detected via the first 30 seconds of audio max
		int detected_lang_id = whisper_lang_auto_detect_with_state(
			m_context,
			whisper_init_state(m_context),
			0, // Offset in milliseconds
			m_config.m_hardware_config.m_thread_count > 0 ? m_config.m_hardware_config.m_thread_count : 4, // TODO: Check if 0 can be used here (for auto detect)
			lang_probs.data()
		);
		
		if (detected_lang_id >= 0) {
			result.m_language_code	= whisper_lang_str(detected_lang_id);
			result.m_confidence		= lang_probs[detected_lang_id];
			result.m_result_code	= EResultCode::Success;
		}

		return result;
	}

	std::string CSTTEngineImpl::ExtractTextTokens(std::vector<float>& a_probabilities, bool a_get_word_probabilities) {
		if (!m_context) { 
			return ""; 
		}
		
		std::string result;
		whisper_token eot_token = whisper_token_eot(m_context);
		int segment_count = whisper_full_n_segments(m_context);

		for (int segment = 0; segment < segment_count; ++segment) {
			if (!a_get_word_probabilities) {
				// Just get segment text & concatenate if it exists
				const char* segment_text = whisper_full_get_segment_text(m_context, segment);
				if (segment_text) {
					result += segment_text;
				}
				continue;
			}

			// Word probabilities are requested, so we iterate through the tokens
			int token_count = whisper_full_n_tokens(m_context, segment);

			for (int token = 0; token < token_count; ++token) {
				whisper_token_data token_data = whisper_full_get_token_data(m_context, segment, token);

				// Skip special tokens (i.e., end-of-text)
				if (token_data.id >= eot_token) {
					continue;
				}

				const char* token_text = whisper_full_get_token_text(m_context, segment, token);
				if (!token_text) {
					continue;
				}

				result += token_text;
				
				// New words start w/ whitespace, so record the probability
				if (token_text[0] != '\0' && std::isspace(static_cast<unsigned char>(token_text[0]))) {
					a_probabilities.push_back(whisper_full_get_token_p(m_context, segment, token));
				}
			}
		}
		return result;
	}

	void CSTTEngineImpl::UpdateWhisperContext(whisper_full_params& a_params, int a_part_index) {
		// Keep context between parts = false
		// Reset on first part only = true
		a_params.no_context = (a_part_index == 0) ? true : false; 
	}

	void CSTTEngineImpl::UpdateWhisperParams(whisper_full_params& a_params, int a_part_index) {
		//const auto& dtw_config				= m_config.m_dtw_config;
		//const auto& system_config			= m_config.m_system_config;
		//const auto& open_vino_config		= m_config.m_open_vino_config;


		// Model settings
		const auto& model_config	= m_config.m_model_config;
		a_params.language			= model_config.m_language.empty() ? nullptr : model_config.m_language.c_str();
		a_params.translate			= model_config.m_translate_to_english;
		a_params.initial_prompt		= model_config.m_initial_prompt.empty() ? nullptr : model_config.m_initial_prompt.c_str();

		// Grammar settings (if enabled)
		const auto& grammar_config = m_config.m_grammar_config;
		if (!grammar_config.m_gbnf_grammar.empty()) {
			a_params.grammar_penalty = grammar_config.m_grammar_penalty;
		}

		// Feature settings
		const auto& feature_config		= m_config.m_feature_config;
		a_params.no_context				= (a_part_index == 0); // Reset on first part only
		a_params.suppress_blank			= feature_config.m_suppress_blank;
		a_params.suppress_nst			= feature_config.m_suppress_nst;
		a_params.suppress_regex			= feature_config.m_suppress_regex.empty() ? nullptr : feature_config.m_suppress_regex.c_str();
		a_params.detect_language		= feature_config.m_detect_language;
		a_params.tdrz_enable			= feature_config.m_tinydiarize_enable;
		a_params.debug_mode				= feature_config.m_debug_mode;

		// Decoder settings
		const auto& decoder_config		= m_config.m_decoder_config;
		a_params.strategy				= Utilities::VoxBoxSamplingStrategyToWhisper(decoder_config.m_sampling_strategy);
		a_params.greedy.best_of			= decoder_config.m_best_of;
		a_params.n_max_text_ctx			= decoder_config.m_max_text_context;
		a_params.max_len				= decoder_config.m_max_length;
		a_params.beam_search.beam_size	= decoder_config.m_beam_size;

		// Audio processing
		const auto& audio_processing_config = m_config.m_audio_processing_config;
		m_progress_tracker.SetProgressStep(audio_processing_config.m_progress_step);
		a_params.offset_ms					= audio_processing_config.m_offset_t_ms;
		a_params.duration_ms				= audio_processing_config.m_duration_ms;

		// Thresholds
		const auto& threshold_config = m_config.m_threshold_config;
		a_params.thold_pt = threshold_config.m_word_thold;
		a_params.entropy_thold = threshold_config.m_entropy_thold;
		a_params.no_speech_thold = threshold_config.m_no_speech_thold;
		a_params.logprob_thold = threshold_config.m_logprob_thold;

		// Temperature
		const auto& temperature_config = m_config.m_temperature_config;
		a_params.temperature = temperature_config.m_temperature;
		a_params.temperature_inc = temperature_config.m_temperature_inc;
		// TODO: no fallback option	

		// Hardware
		const auto& hardware_config = m_config.m_hardware_config;
		a_params.n_threads = (hardware_config.m_thread_count > 0) ? hardware_config.m_thread_count : 0;
		
		// Output
		const auto& output_config	= m_config.m_output_config;
		a_params.print_special		= output_config.m_print_special;
		a_params.print_progress		= output_config.m_print_progress;
		a_params.print_timestamps	= output_config.m_timestamps;
		a_params.token_timestamps	= output_config.m_word_timestamps;
		a_params.split_on_word		= output_config.m_split_on_word;

		// Callbacks
		// We pass the address of the progress tracker class as whisper's user data
		a_params.progress_callback					= Utilities::WhisperProgressCallback;
		a_params.progress_callback_user_data		= &m_progress_tracker;
		a_params.abort_callback						= Utilities::WhisperAbortCallback;
		a_params.abort_callback_user_data			= &m_progress_tracker;
		a_params.encoder_begin_callback				= Utilities::WhisperEncoderBeginCallback;
		a_params.encoder_begin_callback_user_data	= &m_progress_tracker;
	}
}