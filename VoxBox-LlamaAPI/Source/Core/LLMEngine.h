#pragma once
#include "LLMConfig.h"
#include "LLMResult.h"
#include "LLMState.h"
#include "LLMTokenTypes.h"

#include <vector>
#include <memory>
#include <cstdint>

// Forward declarations of llama.cpp types
struct llama_model;
struct llama_context;
struct llama_sampler;
struct llama_model_tensor_buft_override;

namespace VoxBox {

	class CLLMEngineImpl {
	private:
		SLLMConfig m_config;
		std::string m_inference_text; 

		// MoE override storage -- must outlive m_llama_model
		std::vector<std::string> m_moe_patterns;
		std::vector<llama_model_tensor_buft_override> m_moe_overrides;

		// llama.cpp specific structs
		llama_model*	m_llama_model	= nullptr;
		llama_context*	m_llama_context = nullptr;
		llama_sampler*	m_llama_sampler = nullptr;

		// Inference state snapshot 
		std::unique_ptr<SLLMSnapshot> m_context_state_snapshot = nullptr; // We'll do 1 snapshot per engine instance for now
		
		// Current runtime inference state
		int m_token_count = 0;
		int m_last_token_type = 0;

		// Flags
		bool m_is_initialized = false;

	public:
		explicit CLLMEngineImpl(const SLLMConfig& a_config);
		~CLLMEngineImpl();

		// Lifecycle
		void Init(const SLLMConfig& a_config);
		void Reinit(const SLLMConfig& a_config);
		void Shutdown();

		// Results
		SInferenceResult Query(const char* a_prompt, bool a_skip_sys_prompt_end = false, bool a_resume_from_snapshot = false);
		SEmbeddingResult CreateEmbeddings(const char* a_prompt);

		// Token helpers
		int GetTokenCount(const char* a_text, bool a_add_special_char = true);

		// Runtime context state management
		void Reset(const char* a_sys_prompt = nullptr);

		// Serialized context state management
		void SnapshotClear();
		bool SnapshotUpdate();
		bool SnapshotRestore();
		bool SnapshotToFile(const char* a_file_path);
		bool SnapshotFromFile(const char* a_file_path);

		// Getters
		inline bool IsInitialized() const			{ return m_is_initialized; }
		inline const SLLMConfig& GetConfig() const	{ return m_config; }
		
		// Setters
		void SetTokenCallback(TokenCallbackFn a_callback);
		void SetSamplerConfig(const SLLMSamplerConfig& a_sampler_config);
	
	private: // Internal helpers
		void LoadModel();
		void CreateContext();
		void CreateSampler();
		void ClearMemory();																// Clears KV cache

		// Snapshot serialization primitives
		std::unique_ptr<SLLMSnapshot> CreateSnapshot(); 								// Runtime context state serialization
		bool RestoreStateSnapshot(const SLLMSnapshot& a_snapshot);

		// Token decoding
		bool DecodeSingleToken(int32_t a_token, int a_position, bool a_output_logits);	// llama_token = int32_t
		int DecodeString(const char* a_string, int a_token_type);						// Tokenizes & decodes a string into the context

		// Query dispatch
		bool DecodeInitialQuery(const char* a_prompt, bool a_skip_sys_prompt_end);		// System prompt + first user prompt
		bool DecodeFollowUpQuery(const char* a_prompt);									// Subsequent user prompt
		bool DecodePromptAndSysPromptEndDelim(const char* a_prompt);					// Snapshot resume -- Decodes remaining prompt + sys_prompt_end_delim

		// Inference loop
		bool RunInference();															// Samples until EOG

		// Model metadata
		void TryAndSetPromptsFromModel();
		
	};
}

