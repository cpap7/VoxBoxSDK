#include "vblpch.h"
#include "LLMEngine.h"

// llama.cpp includes
#include <llama.h>
#include <common.h> 

namespace VoxBox {
	namespace Utilities {
		static llama_flash_attn_type VBToLlamaFlashAttentionType(EFlashAttentionType a_type) {
			switch (a_type) {
				case EFlashAttentionType::Default:		break;
				case EFlashAttentionType::Disabled:		return LLAMA_FLASH_ATTN_TYPE_DISABLED;
				case EFlashAttentionType::Enabled:		return LLAMA_FLASH_ATTN_TYPE_ENABLED;
			}

			return LLAMA_FLASH_ATTN_TYPE_AUTO; // Default = auto
		}

		static enum llama_pooling_type VBToLlamaPoolingType(EPoolingType a_type) {
			switch (a_type) {
				case EPoolingType::Default:		break;
				case EPoolingType::None:		return LLAMA_POOLING_TYPE_NONE;
				case EPoolingType::Mean:		return LLAMA_POOLING_TYPE_MEAN;
				case EPoolingType::CLS:			return LLAMA_POOLING_TYPE_CLS;
				case EPoolingType::Last:		return LLAMA_POOLING_TYPE_LAST;
				case EPoolingType::Rank:		return LLAMA_POOLING_TYPE_RANK;
			}

			return LLAMA_POOLING_TYPE_UNSPECIFIED; // Default = load from model
		}


		static EFlashAttentionType LlamaToVBFlashAttentionType(llama_flash_attn_type a_type) {
			switch (a_type) {
				case LLAMA_FLASH_ATTN_TYPE_AUTO:		break;
				case LLAMA_FLASH_ATTN_TYPE_DISABLED:	return EFlashAttentionType::Disabled;
				case LLAMA_FLASH_ATTN_TYPE_ENABLED:		return EFlashAttentionType::Enabled;
			}

			return EFlashAttentionType::Default;
		}

		static EPoolingType LlamaToVBPoolingType(enum llama_pooling_type a_type) {
			switch (a_type) {
				case LLAMA_POOLING_TYPE_UNSPECIFIED:	break;
				case LLAMA_POOLING_TYPE_NONE:			return EPoolingType::None;
				case LLAMA_POOLING_TYPE_MEAN:			return EPoolingType::Mean;
				case LLAMA_POOLING_TYPE_CLS:			return EPoolingType::CLS;
				case LLAMA_POOLING_TYPE_LAST:			return EPoolingType::Last;
				case LLAMA_POOLING_TYPE_RANK:			return EPoolingType::Rank;
			}
			return EPoolingType::Default;
		}
	}

	static bool s_llama_backend_initialized = false;
	
	static void InitLlamaBackendIfNecessary() {
		if (!s_llama_backend_initialized) {
			llama_backend_init();
			llama_numa_init(GGML_NUMA_STRATEGY_DISABLED);
			s_llama_backend_initialized = true;
		}
	}

	CLLMEngineImpl::CLLMEngineImpl(const SLLMConfig& a_config) 
		: m_config(a_config) {
		Init(a_config);
	}
	
	CLLMEngineImpl::~CLLMEngineImpl() {
		Shutdown();
	}

	void CLLMEngineImpl::Init(const SLLMConfig& a_config) {
		if (m_is_initialized) {
			Shutdown();
		}
		m_config = a_config; // Update config if it's changed upon reinit

		InitLlamaBackendIfNecessary();

		LoadModel();
		if (!m_llama_model) {
			printf("[VoxBox LLM] Error: Failed to load model!\n");
			Shutdown();
			return;
		}

		// Auto-detect prompt delimiters via model metadata if requested
		if (m_config.m_prompt_config.m_try_prompts_by_model) {
			TryAndSetPromptsFromModel();
		}
		
		CreateContext();
		if (!m_llama_context) {
			printf("[VoxBox LLM] Error: Failed to create context!\n");
			Shutdown();
			return;
		}
		
		CreateSampler();
		if (!m_llama_sampler) {
			printf("[VoxBox LLM] Error: Failed to create sampler!\n");
			Shutdown();
			return;
		}
		
		// Validate context size vs training size
		int32_t n_ctx_train = llama_model_n_ctx_train(m_llama_model);
		int32_t n_ctx_actual = static_cast<int32_t>(llama_n_ctx(m_llama_context));
		if (n_ctx_train < n_ctx_actual && !m_config.m_context_config.m_embeddings) {
			printf("[VoxBox LLM] Error: Model was trained on %d tokens (required = %d tokens).\n", n_ctx_train, n_ctx_actual);
			Shutdown();
			return;
		}

		m_token_count		= 0;
		m_last_token_type	= 0;
		m_is_initialized	= true;
	}

	void CLLMEngineImpl::Reinit(const SLLMConfig& a_config) {
		Shutdown();
		Init(a_config);
	}

	void CLLMEngineImpl::Shutdown() {
		if (m_llama_sampler) {
			llama_sampler_free(m_llama_sampler);
			m_llama_sampler = nullptr;
		}
		
		if (m_llama_context) {
			llama_free(m_llama_context);
			m_llama_context = nullptr;
		}

		if (m_llama_model) {
			llama_model_free(m_llama_model);
			m_llama_model = nullptr;
		}

		// Clear MoE override data
		m_moe_patterns.clear();
		m_moe_overrides.clear();

		m_token_count		= 0;
		m_last_token_type	= 0;
		m_is_initialized	= false;
	}

	SInferenceResult CLLMEngineImpl::Query(const char* a_prompt, bool a_skip_sys_prompt_end, bool a_resume_from_snapshot) {
		SInferenceResult result;

		if (!m_is_initialized) {
			result.m_result_code = EResultCode::NotInitialized;
			return result;
		}

		if (!a_prompt || a_prompt[0] == '\0') {
			result.m_result_code = EResultCode::InvalidParameter;
			return result;
		}

		if (m_config.m_context_config.m_embeddings) {
			result.m_result_code = EResultCode::InvalidParameter;
			return result;
		}

		// Decode prompt into context
		bool decode_success = false;
		if (m_token_count == 0 && !m_config.m_prompt_config.m_sys_prompt.empty()) {
			decode_success = DecodeInitialQuery(a_prompt, a_skip_sys_prompt_end);
		}
		else if (a_resume_from_snapshot) {
			decode_success = DecodePromptAndSysPromptEndDelim(a_prompt);
		}
		else {
			decode_success = DecodeFollowUpQuery(a_prompt);
		}

		if (!decode_success) {
			result.m_result_code = EResultCode::ProcessingFailed;
			return result;
		}

		// Sample tokens until EOG
		if (!a_skip_sys_prompt_end) {
			if (!RunInference()) {
				result.m_result_code = EResultCode::ProcessingFailed;
				return result;
			}
		}

		result.m_text = m_inference_text; // Populated via the internal functions
		result.m_result_code = EResultCode::Success;
		return result;
	}

	SEmbeddingResult CLLMEngineImpl::CreateEmbeddings(const char* a_prompt) {
		SEmbeddingResult result;

		if (!m_is_initialized) {
			result.m_result_code = EResultCode::NotInitialized;
			return result;
		}

		if (!a_prompt || a_prompt[0] == '\0') {
			result.m_result_code = EResultCode::InvalidParameter;
			return result;
		}

		if (m_config.m_context_config.m_embeddings) {
			result.m_result_code = EResultCode::InvalidParameter;
			return result;
		}
		
		// Clear KV cache
		ClearMemory();

		// Tokenize
		std::vector<int32_t> tokens = common_tokenize(m_llama_context, a_prompt, true, true);
		if (tokens.empty()) {
			result.m_result_code = EResultCode::ProcessingFailed;
			return result;
		}

		uint32_t n_ctx = llama_n_ctx(m_llama_context);
		if (n_ctx < static_cast<int32_t>(tokens.size())) {
			result.m_result_code = EResultCode::InvalidParameter;
			return result;
		}

		// Batch-decode all tokens
		int32_t token_batch_size = static_cast<int32_t>(tokens.size());
		llama_batch batch = llama_batch_init(token_batch_size, 0, 1);
		
		for (int32_t i{}; i < token_batch_size; ++i) {
			common_batch_add(batch, tokens[i], i, { 0 }, true);
		}
		
		int32_t llama_decode_result = llama_decode(m_llama_context, batch); // 0 = success
		llama_batch_free(batch);
		
		if (llama_decode_result != 0) {
			result.m_result_code = EResultCode::ProcessingFailed;
			return result;
		}

		// Extract embeddings vector
		const float* embeddings = llama_get_embeddings_seq(m_llama_context, 0);
		if (!embeddings) {
			result.m_result_code = EResultCode::ProcessingFailed;
			return result;
		}
		
		size_t n_embeddings = static_cast<size_t>(llama_model_n_embd(m_llama_model));

		// Euclidean-normalize
		result.m_embeddings.resize(n_embeddings);
		common_embd_normalize(embeddings, result.m_embeddings.data(), n_embeddings, 2);
		
		result.m_result_code = EResultCode::Success;
		return result;
	}

	int CLLMEngineImpl::GetTokenCount(const char* a_text, bool a_add_special_char) {
		if (!m_is_initialized || !a_text) {
			return -1;
		}

		if (m_config.m_context_config.m_embeddings) {
			return -2; // Invalid for embedding mode
		}

		const llama_vocab* vocab = llama_model_get_vocab(m_llama_model);
		std::vector<int32_t> tokens = common_tokenize(vocab, a_text, a_add_special_char, true);
		
		int token_count = static_cast<int>(tokens.size());

		return token_count;
	}

	void CLLMEngineImpl::Reset(const char* a_sys_prompt) {
		// For state management //

		if (!m_is_initialized) {
			return;
		}
		ClearMemory();
		llama_sampler_reset(m_llama_sampler);

		m_token_count		= 0;
		m_last_token_type	= 0;
		m_inference_text.clear();

		if (a_sys_prompt) {
			m_config.m_prompt_config.m_sys_prompt = a_sys_prompt;
		}
	}

	void CLLMEngineImpl::SetTokenCallback(TokenCallbackFn a_callback) {
		m_config.m_token_callback_function = std::move(a_callback);
	}
	
	void CLLMEngineImpl::SetSamplerConfig(const SLLMSamplerConfig& a_sampler_config) {
		m_config.m_sampler_config = a_sampler_config;
		
		// Rebuild sampler chain w/ new config if engine is already initialized & sampler is built
		if (m_is_initialized) {
			if (m_llama_sampler) {
				llama_sampler_free(m_llama_sampler);
				m_llama_sampler = nullptr;
			}
			CreateSampler();
		}

	}

	void CLLMEngineImpl::LoadModel() {
		const auto& vb_model_cfg = m_config.m_model_config;
		llama_model_params model_params = llama_model_default_params();
		
		model_params.n_gpu_layers	= vb_model_cfg.m_n_gpu_layers;
		model_params.use_mmap		= vb_model_cfg.m_use_mlock;
		model_params.use_mmap		= vb_model_cfg.m_use_mmap;

		// Build MoE tensor buffer type overrides
		m_moe_patterns.clear();
		m_moe_overrides.clear();

		if (vb_model_cfg.m_n_cpu_moe > 0) {
			// Keep first N MoE layers on CPU
			// This will override m_cpu_moe
			m_moe_patterns.reserve(static_cast<size_t>(vb_model_cfg.m_n_cpu_moe));
			m_moe_overrides.reserve(static_cast<size_t>(vb_model_cfg.m_n_cpu_moe) + 1);

			for (int i{}; i < vb_model_cfg.m_n_cpu_moe; ++i) {
				m_moe_patterns.push_back(llm_ffn_exps_block_regex(i));
				m_moe_overrides.push_back({ m_moe_patterns.back().c_str(), ggml_backend_cpu_buffer_type() });
			}
		}
		else if (vb_model_cfg.m_cpu_moe) {
			// Keep ALL MoE weights on CPU
			m_moe_overrides.push_back(llm_ffn_exps_cpu_override());
		}

		// Null-terminate overrides vector
		m_moe_overrides.push_back({ nullptr, nullptr });
		model_params.tensor_buft_overrides = m_moe_overrides.data();

		m_llama_model = llama_load_model_from_file(vb_model_cfg.m_model_file_path.c_str(), model_params);
	}

	void CLLMEngineImpl::CreateContext() {
		if (!m_llama_model) {
			return;
		}

		const auto& vb_ctx_cfg = m_config.m_context_config;
		llama_context_params ctx_params = llama_context_default_params();

		// Update # of threads that can be used
		int32_t n_threads = vb_ctx_cfg.m_n_threads;
		if (n_threads <= 0) {
			n_threads = static_cast<int32_t>(cpu_get_num_physical_cores());
		}
		ctx_params.n_threads		= n_threads;
		ctx_params.n_threads_batch	= (vb_ctx_cfg.m_threads_batch > 0) ? vb_ctx_cfg.m_threads_batch : n_threads;
		ctx_params.n_seq_max		= vb_ctx_cfg.m_n_seq_max;
		ctx_params.flash_attn_type	= Utilities::VBToLlamaFlashAttentionType(vb_ctx_cfg.m_flash_attention_type);
		ctx_params.offload_kqv		= vb_ctx_cfg.m_offload_kqv;
		
		if (!vb_ctx_cfg.m_embeddings) {
			// Inferencing mode
			ctx_params.n_ctx	= vb_ctx_cfg.m_n_ctx; // 0 = load from model
			ctx_params.n_batch	= vb_ctx_cfg.m_n_batch;
			ctx_params.n_ubatch = vb_ctx_cfg.m_n_ubatch;
		}
		else {
			// Embeddings mode
			ctx_params.embeddings = true;

			int32_t n_ctx_train = static_cast<uint32_t>(llama_model_n_ctx_train(m_llama_model));
			if (vb_ctx_cfg.m_n_ctx == 0 || vb_ctx_cfg.m_n_ctx < n_ctx_train) {
				ctx_params.n_ctx = n_ctx_train;
			}
			else {
				ctx_params.n_ctx = vb_ctx_cfg.m_n_ctx;
			}

			// For pooling, full-context batches are required
			ctx_params.n_batch = ctx_params.n_ctx;
			ctx_params.n_ubatch = ctx_params.n_ctx;

			// Pooling type from model will be derived from either the model or an explicitly set value
			auto pooling_type = Utilities::VBToLlamaPoolingType(vb_ctx_cfg.m_pooling_type);
			if (pooling_type == LLAMA_POOLING_TYPE_UNSPECIFIED || pooling_type == LLAMA_POOLING_TYPE_NONE) {
				pooling_type = LLAMA_POOLING_TYPE_MEAN; 
			}
			ctx_params.pooling_type = pooling_type;
		}
		m_llama_context = llama_init_from_model(m_llama_model, ctx_params);
	}

	void CLLMEngineImpl::CreateSampler() {
		if (!m_llama_model) {
			return;
		}

		const auto& vb_samp_cfg = m_config.m_sampler_config;
		const llama_vocab* vocab = llama_model_get_vocab(m_llama_model);
		llama_sampler_chain_params samp_chain_cfg = llama_sampler_chain_default_params();
		llama_sampler* chain = llama_sampler_chain_init(samp_chain_cfg);

		// Grammar must be first in chain if present
		if (!vb_samp_cfg.m_grammar.empty()) {
			llama_sampler* grammar = llama_sampler_init_grammar(vocab, vb_samp_cfg.m_grammar.c_str(), "root");
			if (grammar) {
				llama_sampler_chain_add(chain, grammar);
			}
		}
		
		// Add sampling stages
		llama_sampler_chain_add(chain, llama_sampler_init_top_k(vb_samp_cfg.m_top_k));
		llama_sampler_chain_add(chain, llama_sampler_init_top_p(vb_samp_cfg.m_top_p, 0));
		llama_sampler_chain_add(chain, llama_sampler_init_min_p(vb_samp_cfg.m_min_p, 0));
		llama_sampler_chain_add(chain, llama_sampler_init_temp(vb_samp_cfg.m_temp));
		llama_sampler_chain_add(chain, llama_sampler_init_dist(vb_samp_cfg.m_seed));

		m_llama_sampler = chain;
	}

	void CLLMEngineImpl::ClearMemory() {
		llama_memory_t memory = llama_get_memory(m_llama_context);
		if (memory) {
			llama_memory_clear(memory, true);
		}
	}

	bool CLLMEngineImpl::DecodeSingleToken(int32_t a_token, int a_position, bool a_output_logits) {
		llama_batch batch = llama_batch_init(1, 0, 1);

		common_batch_add(batch, a_token, a_position, { 0 }, a_output_logits);
		
		if (llama_decode(m_llama_context, batch) != 0) {
			llama_batch_free(batch);
			return false;
		}

		llama_batch_free(batch);
		llama_sampler_accept(m_llama_sampler, a_token);
		return true;
	}

	int CLLMEngineImpl::DecodeString(const char* a_string, int a_token_type) {
		const llama_vocab* vocab = llama_model_get_vocab(m_llama_model);
		std::vector<int32_t> tokens = common_tokenize(vocab, a_string, false, true);

		// Prepend BOS if context is empty and the model requires it
		if (m_token_count == 0 && llama_vocab_get_add_bos(vocab)) {
			tokens.insert(tokens.begin(), llama_vocab_bos(vocab));
		}

		m_last_token_type = a_token_type;
		int32_t count = static_cast<int32_t>(tokens.size());
		for (int32_t i{}; i < count; ++i) {
			bool is_last = (i + 1 == count);

			if (!DecodeSingleToken(tokens[i], m_token_count + i, is_last)) {
				return -1;
			}

			// Execute callback for each token
			if (m_config.m_token_callback_function) {
				std::string token_piece = common_token_to_piece(m_llama_context, tokens[i], true);
				if (!token_piece.empty()) {
					m_config.m_token_callback_function(static_cast<int>(tokens[i]), token_piece.c_str(), m_last_token_type, nullptr);
				}
			}	
		}
		m_token_count += count; // Track internally
		return count;

	}

	bool CLLMEngineImpl::DecodeInitialQuery(const char* a_prompt, bool a_skip_sys_prompt_end) {
		const auto& vb_prompt_cfg = m_config.m_prompt_config;
		
		if (DecodeString(vb_prompt_cfg.m_sys_prompt_beg_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
			return false;
		}
		
		if (DecodeString(vb_prompt_cfg.m_sys_prompt.c_str(), static_cast<int>(ETokenType::SystemPrompt)) < 0) {
			return false;
		}
		
		if (DecodeString(vb_prompt_cfg.m_sys_prompt_mid_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
			return false;
		}
		
		if (DecodeString(a_prompt, static_cast<int>(ETokenType::Prompt)) < 0) {
			return false;
		}

		if (!a_skip_sys_prompt_end) {
			if (DecodeString(vb_prompt_cfg.m_sys_prompt_end_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
				return false;
			}
		}

		return true;
	}
	
	bool CLLMEngineImpl::DecodeFollowUpQuery(const char* a_prompt) {
		const auto& vb_prompt_cfg = m_config.m_prompt_config;

		if (DecodeString(vb_prompt_cfg.m_prompt_beg_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
			return false;
		}

		if (DecodeString(a_prompt, static_cast<int>(ETokenType::Prompt)) < 0) {
			return false;
		}

		if (DecodeString(vb_prompt_cfg.m_prompt_end_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
			return false;
		}
		
		return true;
	}

	bool CLLMEngineImpl::DecodePromptAndSysPromptEndDelim(const char* a_prompt) {
		const auto& vb_prompt_cfg = m_config.m_prompt_config;

		if (DecodeString(a_prompt, static_cast<int>(ETokenType::Prompt)) < 0) {
			return false;
		}

		if (DecodeString(vb_prompt_cfg.m_sys_prompt_end_delim.c_str(), static_cast<int>(ETokenType::Delimiter)) < 0) {
			return false;
		}

		return true;
	}

	bool CLLMEngineImpl::RunInference() {
		const auto& vb_prompt_cfg = m_config.m_prompt_config;
		const llama_vocab* vocab = llama_model_get_vocab(m_llama_model);
		
		int n_ctx = static_cast<int>(llama_n_ctx(m_llama_context));
		
		bool is_thinker = !vb_prompt_cfg.m_think_beg_delim.empty();
		bool is_thinking = (m_last_token_type == static_cast<int>(ETokenType::ThinkBegin) || m_last_token_type == static_cast<int>(ETokenType::ThinkText));
		bool irq = false; // Interrupt request
		
		m_inference_text.clear();
		llama_batch batch = llama_batch_init(1, 0, 1);

		for (int n_current = m_token_count; n_current < n_ctx; ++n_current) {
			if (irq) {
				// Decode interrupt tokens -- EOT/EOS
				llama_token eot_token = llama_vocab_eot(vocab);
				llama_token irq_token = (eot_token != -1) ? eot_token : llama_vocab_eos(vocab);

				m_last_token_type = static_cast<int>(ETokenType::InterruptRequest);

				common_batch_clear(batch);
				common_batch_add(batch, irq_token, n_current, { 0 }, false);

				if (llama_decode(m_llama_context, batch) != 0) {
					llama_batch_free(batch);
					return false;
				}

				llama_sampler_accept(m_llama_sampler, irq_token);
				
				++n_current;
				break;
			}

			// Sample next token
			llama_token new_token = llama_sampler_sample(m_llama_sampler, m_llama_context, -1);
			bool is_eog = llama_vocab_is_eog(vocab, new_token);
			bool is_control = llama_vocab_is_control(vocab, new_token);
			std::string piece = common_token_to_piece(m_llama_context, new_token, true);

			// Resolve current token type
			// TODO: Clean this up
			int token_type = 0;
			if (is_eog) {
				token_type = static_cast<int>(ETokenType::SampledEOG);
			}
			
			else if (is_thinker && piece == vb_prompt_cfg.m_think_beg_delim) {
				token_type = static_cast<int>(ETokenType::ThinkBegin);
				is_thinking = true;
			}

			else if (is_thinker && piece == vb_prompt_cfg.m_think_end_delim) {
				token_type = static_cast<int>(ETokenType::ThinkEnd);
			}
			
			else if (is_control) {
				token_type = static_cast<int>(ETokenType::SampledCtrlNonEOG);
			}
			
			else if (is_thinking) {
				token_type = static_cast<int>(ETokenType::ThinkText);
			}

			else {
				token_type = static_cast<int>(ETokenType::SampledNonEOGNonCtrl);
				m_inference_text += piece;
			}
			m_last_token_type = token_type;
			
			// Execute callback
			// return true = interrupt requested
			if (m_config.m_token_callback_function && !piece.empty()) {
				irq = m_config.m_token_callback_function(
					static_cast<int>(new_token),
					piece.c_str(),
					token_type,
					nullptr
				);
			}

			// is_thinking should be false after callback see ThinkEnd
			if (is_thinker && token_type == static_cast<int>(ETokenType::ThinkEnd)) {
				is_thinking = false;
			}

			// Decode new sampled token
			common_batch_clear(batch);
			common_batch_add(batch, new_token, n_current, { 0 }, true);

			if (llama_decode(m_llama_context, batch) != 0) {
				llama_batch_free(batch);
				return false;
			}
			llama_sampler_accept(m_llama_sampler, new_token);

			if (is_eog) {
				++n_current;
				break;
			}

		}
		
		llama_batch_free(batch);
		m_token_count = static_cast<int>(llama_memory_seq_pos_max(llama_get_memory(m_llama_context), 0)) + 1;
		return true;
	}

	void CLLMEngineImpl::TryAndSetPromptsFromModel() {
		// TODO
	}
}