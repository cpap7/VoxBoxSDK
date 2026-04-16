#pragma once
#include <VBCommon.h>

#ifdef __cplusplus
	#include <cstdint>
	#include <cstdbool>
	#include <string>
	#include <vector>
	#include <memory>
	#include <functional>
	
	#include "../Core/LLMConfig.h"
	#include "../Core/LLMResult.h"
	#include "../Core/LLMState.h"
	#include "../Core/LLMTokenTypes.h"

#else
	#include <stdint.h>
	#include <stdbool.h>
#endif

#define VB_LLM_VERSION "1.0.0"

// C API
#ifdef __cplusplus
extern "C" {
#endif

	/* Opaque handle for C */
	
	// Engine handle
	typedef struct VB_LLM_Handle_t* VB_LLM_EngineHandle_t;

	// C config struct (for interop)
	// See Core/LLMConfig.h for more details on what each member is responsible for
	typedef struct {
		const char* m_model_file_path;
		const char* m_grammar;			

		// Prompt template + delimiters
		const char* m_sys_prompt;		// e.g., "You are a helpful AI agent"
		const char* m_sys_prompt_beg_delim;
		const char* m_sys_prompt_mid_delim;
		const char* m_sys_prompt_end_delim;
		const char* m_prompt_beg_delim;
		const char* m_prompt_end_delim;
		const char* m_think_beg_delim;
		const char* m_think_end_delim;
		
		uint32_t m_n_ctx;				//  0 = Model default
		uint32_t m_n_batch;				//  1 = Default; Logical max batch size
		uint32_t m_n_ubatch;			//  1 = Default; Physical max batch size
		uint32_t m_n_seq_max;			//  1 = Default; Max sequences (recurrent models)
		uint32_t m_seed;				// -1 = Random

		int32_t m_n_threads;			// -1 = Auto
		int32_t m_n_gpu_layers;			// -1 = Offload all
		int32_t m_n_cpu_moe;			//  0 = Default; Keep 1st N MoE layers within CPU (which will override m_cpu_moe)
		int32_t m_threads_batch;		//  0 = Default; Threads for batch processing
		int32_t m_flash_attention;		// -1 = Auto, 0 = Disabled, 1 = Enabled
		int32_t m_pooling_type;			// -1 = Unspecified (derive from model)
		int32_t m_top_k;

		float m_top_p;
		float m_min_p;
		float m_temp;

		int m_cpu_moe;					// 0 (or false) = default; Keep ALL MoE weights within CPU
		int m_try_prompts_by_model;		// 0 (or false) = default
		int m_embeddings;				// 0 (or false) = default
		int m_use_mlock;				// 0 (or false) = default
		int m_use_mmap;					// 1 (or true)  = default
		int m_offload_kqv;				// 1 (or true)  = default; Offload KQV to GPU

	} VB_LLM_Config_t;

	typedef struct {
		char* m_text;					// Caller must free via VB_LLM_FreeString
		int m_success;		
	} VB_LLM_InferenceResult_t;

	typedef struct {
		float* m_embeddings;			// Caller must free via VB_LLM_FreeFloatArray
		int m_count;
		int m_success;
	} VB_LLM_EmbeddingResult_t;

	// Token callback
	// Return 0 to stop inference
	typedef int (*VB_LLM_TokenCallbackFn)(int a_token_id, const char* a_token_text, int a_token_type, const float* a_digit_probabilities);

	// Lifecycle
	VB_LLM_API VB_LLM_EngineHandle_t VB_CALL VB_LLM_Create(const VB_LLM_Config_t* a_config);
	VB_LLM_API void VB_CALL VB_LLM_Destroy(VB_LLM_EngineHandle_t a_engine_handle);	// For deleting engine instances based on the engine handle
	VB_LLM_API int VB_CALL VB_LLM_Reinit(VB_LLM_EngineHandle_t a_engine_handle, const VB_LLM_Config_t* a_config);
	VB_LLM_API int VB_CALL VB_LLM_IsInitialized(VB_LLM_EngineHandle_t a_engine_handle);

	// Inferences
	VB_LLM_API VB_LLM_InferenceResult_t VB_CALL VB_LLM_Query(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt, int a_skip_sys_prompt_end, int a_resume_from_snapshot);
	VB_LLM_API const char* VB_CALL VB_LLM_QuerySimple(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt);

	// Embeddings
	VB_LLM_API VB_LLM_EmbeddingResult_t VB_CALL VB_LLM_CreateEmbeddings(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt);

	// Token utilities
	VB_LLM_API int VB_CALL VB_LLM_GetTokenCount(VB_LLM_EngineHandle_t a_engine_handle, const char* a_text, int a_add_special);

	// Control
	VB_LLM_API void VB_CALL VB_LLM_Reset(VB_LLM_EngineHandle_t a_engine_handle, const char* a_sys_prompt);
	VB_LLM_API void VB_CALL VB_LLM_SetTokenCallback(VB_LLM_EngineHandle_t a_engine_handle, VB_LLM_TokenCallbackFn a_callback);

	// Snapshot management
	// NOTE: returning 1 == success, returning 0 == failure
	VB_LLM_API void VB_CALL VB_LLM_SnapshotClear(VB_LLM_EngineHandle_t a_engine_handle);
	VB_LLM_API int VB_CALL VB_LLM_SnapshotUpdate(VB_LLM_EngineHandle_t a_engine_handle);
	VB_LLM_API int VB_CALL VB_LLM_SnapshotRestore(VB_LLM_EngineHandle_t a_engine_handle);
	VB_LLM_API int VB_CALL VB_LLM_SnapshotToFile(VB_LLM_EngineHandle_t a_engine_handle, const char* a_file_path);
	VB_LLM_API int VB_CALL VB_LLM_SnapshotFromFile(VB_LLM_EngineHandle_t a_engine_handle, const char* a_file_path);

	// Memory management
	VB_LLM_API void VB_CALL VB_LLM_FreeString(const char* a_string);
	VB_LLM_API void VB_CALL VB_LLM_FreeFloatArray(float* a_float_array);
	VB_LLM_API void VB_CALL VB_LLM_FreeInferenceResult(VB_LLM_InferenceResult_t* a_inference_result);
	VB_LLM_API void VB_CALL VB_LLM_FreeEmbeddingResult(VB_LLM_EmbeddingResult_t* a_embedding_result);

	// Utility
	VB_LLM_API VB_LLM_Config_t VB_CALL VB_LLM_GetDefaultConfig();
	VB_LLM_API const char* VB_CALL VB_LLM_GetVersion();

#ifdef __cplusplus
}
#endif

// C++ API
#ifdef __cplusplus
namespace VoxBox {
	// Forward declaration
	class CLLMEngineImpl;

	// C++ wrapper class for the core engine class
	// Serves as API call dispatcher & has its own factory
	class VB_LLM_API CLLMEngine {
	private:
		std::unique_ptr<CLLMEngineImpl> m_llm_engine = nullptr;

	public:
		// Lifecycle
		CLLMEngine(const SLLMConfig& a_config);
		~CLLMEngine();

		// Don't need copies
		CLLMEngine(const CLLMEngine&) = delete;
		void operator=(const CLLMEngine&) = delete;

		CLLMEngine(CLLMEngine&& a_other) noexcept;
		void operator=(CLLMEngine&& a_other) noexcept;
	
		// Status
		bool IsInitialized() const;
		explicit operator bool() const { return IsInitialized(); }
		
		// Inference
		SInferenceResult Query(const std::string& a_prompt, bool a_skip_sys_prompt_end = false, bool a_resume_from_snapshot = false);

		// Embeddings
		SEmbeddingResult CreateEmbeddings(const std::string& a_prompt);

		// Token utils
		int GetTokenCount(const std::string& a_text, bool a_add_special_char = true);
		void SetTokenCallback(TokenCallbackFn a_callback);

		// Sampler setters
		void SetSamplerConfig(const SLLMSamplerConfig& a_config);
		void SetSamplerGrammar(std::string& a_grammar);
		void SetSeed(uint32_t a_seed = -1);				// -1 = random; 0xFFFFFFFF = LLAMA_DEFAULT_SEED;
		void SetTopK(int32_t a_top_k = 0);				// (<= 0) = vocab size
		void SetTopP(float a_top_p = 1.0f);				// 1.0f = disabled
		void SetMinP(float a_min_p = 0.0f);				// 0.0f = disabled
		void SetTemperature(float a_temp = 0.0f);		// Temp <= 0.0 = sample greedily & do not output probability

		// Model config setters
		//void SetModelPath(const std::string& a_model_path = "");
		//void SetGPULayers(int32_t a_n_gpu_layers = -1);
		//void SetCPUMoE(int32_t a_n_cpu_moe = 0);
		//void UseCPUMoE(bool a_flag = false);
		//void UseMMap(bool a_flag = true);
		//void UseMLock(bool a_flag = false);

		// Control
		void Reinit(const SLLMConfig& a_config);
		void Reset(const std::string& a_sys_prompt);

		// Context state snapshot management
		void SnapshotClear();
		bool SnapshotUpdate();
		bool SnapshotRestore();
		bool SnapshotToFile(const std::string& a_file_path);
		bool SnapshotFromFile(const std::string& a_file_path);

		// Factory
		static std::unique_ptr<CLLMEngine> Create(const SLLMConfig& a_config);

		// Misc
		static SLLMConfig GetDefaultConfig();
		static const char* GetVersion();

	};


}
#endif
