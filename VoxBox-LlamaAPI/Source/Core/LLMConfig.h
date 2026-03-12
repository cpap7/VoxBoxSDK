#pragma once

#include <cstdint>
#include <string>
#include <functional>

namespace VoxBox {
	// Model loading params struct (maps to llama_model_params)
	struct SLLMModelConfig {
		std::string m_model_file_path	= "";
		int32_t m_n_gpu_layers			= -1;		// -1 = offload all layers to GPU

		// Mixture of Experts (MoE) CPU offloading --> for building llama_model_params.tensor_buft_overrides
		int32_t m_n_cpu_moe				= 0;		// Keep 1st N MoE layers within CPU (which will override m_cpu_moe)
		bool m_cpu_moe					= false;	// Keep ALL MoE weights within CPU

		bool m_use_mmap					= true;
		bool m_use_mlock				= false;
	};

	enum class EFlashAttentionType : int8_t {
		Auto		= -1,
		Disabled,
		Enabled,

		Default		= Auto	// Default = auto detect
	};

	// Used for embeddings
	enum class EPoolingType : int8_t {
		Unspecified = -1, // Unspecified = derive from model
		None,
		Mean,
		CLS,
		Last,
		Rank,

		Default		= Unspecified // Default = derive from model
	};

	// LLM context (based on llama_context_default_params)
	struct SLLMContextConfig {
		uint32_t m_n_ctx		= 0; // 0 = from model
		uint32_t m_n_batch		= 1; // Logical max batch size 
		uint32_t m_n_ubatch		= 1; // Physical max batch size
		uint32_t m_n_seq_max	= 1; // Max # of sequences (i.e., distinct states for recurrent models)
		
		int32_t m_n_threads		= -1; // # of threads to use for generation
		int32_t m_threads_batch = 0;  // # of threads to use for batch processing
		
		EFlashAttentionType m_flash_attention_type	= EFlashAttentionType::Default;
		EPoolingType m_pooling_type					= EPoolingType::Default;

		bool m_embeddings	= false;	// If true, extract embeddings (together w/ logits)
		bool m_offload_kqv	= true;		// Offload the KQV ops (including the KV cache) to GPU
	};

	struct SLLMSamplerConfig {
		std::string m_grammar	= "";
		uint32_t m_seed			= -1;		// -1 = random; 0xFFFFFFFF = LLAMA_DEFAULT_SEED;
		int32_t m_top_k			= 0;		// (<= 0) = vocab size
		float m_top_p			= 1.0f;		// 1.0f = disabled
		float m_min_p			= 0.0f;		// 0.0f = disabled
		float m_temp			= 0.0f;     // temp <= 0.0 to sample greedily, 0.0 to not output probabil.
	};

	struct SLLMPromptConfig {
		std::string m_sys_prompt			= "";			// The actual system prompt text
		std::string m_sys_prompt_beg_delim	= "";			// e.g. "<|im_start|>system\n"
		std::string m_sys_prompt_mid_delim	= "";			// e.g. "<|im_end|>\n<|im_start|>user\n"
		std::string m_sys_prompt_end_delim	= "";			// e.g. "<|im_end|>\n<|im_start|>assistant\n"
		std::string m_prompt_beg_delim		= "";			// e.g. "<|im_start|>user\n"
		std::string m_prompt_end_delim		= "";			// e.g. "<|im_end|>\n<|im_start|>assistant\n"
		std::string m_think_beg_delim		= "";			// e.g. "<think>"
		std::string m_think_end_delim		= "";			// e.g. "</think>"
		bool m_try_prompts_by_model			= false;		// Auto-detect delimiters from model metadata
	};

	// Token callback: (a_token_id, a_token_text, a_token_type, a_digit_probs = nullptr)
	// Return false to interrupt inference
	using TokenCallbackFn = std::function<bool(int, const char*, int, const float*)>;
	
	//////////////////////////////////////////////////////

	// Top-level config struct for interfacing with llama.cpp
	// Used by the API class CLLMEngine (see VoxBoxLLM.h) 
	// and the internal core class (see LLMEngine.h)
	struct VB_LLM_API SLLMConfig {
		SLLMPromptConfig m_prompt_config;
		TokenCallbackFn m_token_callback_function = nullptr;

		SLLMSamplerConfig m_sampler_config;
		SLLMModelConfig m_model_config;
		SLLMContextConfig m_context_config;
	};
}