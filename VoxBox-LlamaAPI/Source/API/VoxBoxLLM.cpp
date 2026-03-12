#include "vblpch.h"
#include "VoxBoxLLM.h"
#include "../Core/LLMEngine.h"

#include <llama.h>


static VoxBox::SLLMConfig ConvertLLMConfig(const VB_LLM_Config_t* a_config) {
	VoxBox::SLLMConfig config;
	
	if (a_config) {
		// Prompt config
		config.m_prompt_config.m_sys_prompt				= a_config->m_sys_prompt			? a_config->m_sys_prompt			: "";
		config.m_prompt_config.m_sys_prompt_beg_delim	= a_config->m_sys_prompt_beg_delim	? a_config->m_sys_prompt_beg_delim	: "";
		config.m_prompt_config.m_sys_prompt_mid_delim	= a_config->m_sys_prompt_mid_delim	? a_config->m_sys_prompt_mid_delim	: "";
		config.m_prompt_config.m_sys_prompt_end_delim	= a_config->m_sys_prompt_end_delim	? a_config->m_sys_prompt_end_delim	: "";
		config.m_prompt_config.m_prompt_beg_delim		= a_config->m_prompt_beg_delim		? a_config->m_prompt_beg_delim		: "";
		config.m_prompt_config.m_prompt_end_delim		= a_config->m_prompt_end_delim		? a_config->m_prompt_end_delim		: "";
		config.m_prompt_config.m_think_beg_delim		= a_config->m_think_beg_delim		? a_config->m_think_beg_delim		: "";
		config.m_prompt_config.m_think_end_delim		= a_config->m_think_end_delim		? a_config->m_think_end_delim		: "";
		config.m_prompt_config.m_try_prompts_by_model	= a_config->m_try_prompts_by_model != 0;

		// Sampler config
		config.m_sampler_config.m_seed			= a_config->m_seed;
		config.m_sampler_config.m_top_k			= a_config->m_top_k;
		config.m_sampler_config.m_top_p			= a_config->m_top_p;
		config.m_sampler_config.m_min_p			= a_config->m_min_p;
		config.m_sampler_config.m_temp			= a_config->m_temp;
		config.m_sampler_config.m_grammar		= a_config->m_grammar ? a_config->m_grammar : "";
		
		// Model config
		config.m_model_config.m_model_file_path = a_config->m_model_file_path ? a_config->m_model_file_path : "";
		config.m_model_config.m_n_gpu_layers	= a_config->m_n_gpu_layers;
		// TODO: Add & more config params for model config here

		// Context config
		config.m_context_config.m_n_ctx			= a_config->m_n_ctx;
		config.m_context_config.m_n_threads		= static_cast<int>(a_config->m_n_threads);
		config.m_context_config.m_embeddings	= a_config->m_embeddings != 0;
	}

	return config;
}

// Helper to duplicate a std::string into a malloc'd C string (caller frees via VB_LLM_FreeString)
static char* DuplicateString(const std::string& a_string) {
	if (a_string.empty()) {
		return nullptr;
	}

	char* copy = static_cast<char*>(malloc(a_string.size() + 1));
	if (!copy) {
		return nullptr;
	}

	std::memcpy(copy, a_string.c_str(), a_string.size() + 1);
	return copy;
}

VB_LLM_API VB_LLM_EngineHandle_t VB_CALL VB_LLM_Create(const VB_LLM_Config_t* a_config) {
	if (!a_config) {
		printf("[VoxBox LLM] Error: Invalid config.\n");
		return nullptr;
	}

	if (!a_config->m_model_file_path) {
		printf("[VoxBox LLM] Error: Config is missing model file path.\n");
		return nullptr;
	}

	try {
		VoxBox::SLLMConfig config = ConvertLLMConfig(a_config);
		auto* llm_engine = new VoxBox::CLLMEngineImpl(config);
		return reinterpret_cast<VB_LLM_EngineHandle_t>(llm_engine);
	}
	catch (...) {
		return nullptr;
	}
}

VB_LLM_API void VB_CALL VB_LLM_Destroy(VB_LLM_EngineHandle_t a_engine_handle) {
	if (a_engine_handle) {
		auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
		delete llm_engine;
		//llm_engine = nullptr; // Not needed in retrospect; handle goes out of scope of the function anyway
	}
}

VB_LLM_API int VB_CALL VB_LLM_Reinit(VB_LLM_EngineHandle_t a_engine_handle, const VB_LLM_Config_t* a_config) {
	if (!a_engine_handle || !a_config) {
		printf("[VoxBox LLM] Invalid engine handle and/or config.\n");
		return 0; // TODO: Error status codes based on conventions for POSIX maybe
	}
	
	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	VoxBox::SLLMConfig config = ConvertLLMConfig(a_config);
	llm_engine->Reinit(config);
	return llm_engine->IsInitialized() ? 1 : 0;
}

VB_LLM_API int VB_CALL VB_LLM_IsInitialized(VB_LLM_EngineHandle_t a_engine_handle) {
	if (!a_engine_handle) {
		return 0;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->IsInitialized() ? 1 : 0;
}

VB_LLM_API VB_LLM_InferenceResult_t VB_CALL VB_LLM_Query(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt, int a_skip_sys_prompt_end, int a_resume_from_snapshot) {
	VB_LLM_InferenceResult_t result = { nullptr, 0 };
	
	if (!a_engine_handle || !a_prompt) {
		printf("[VoxBox LLM] Invalid engine handle and/or missing prompt.\n");
		return result;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	VoxBox::SInferenceResult cpp_result = llm_engine->Query(a_prompt, a_skip_sys_prompt_end != 0, a_resume_from_snapshot != 0);
	if (cpp_result.Success()) {
		result.m_text		= DuplicateString(cpp_result.m_text);
		result.m_success	= 1;
	}

	return result;
}

VB_LLM_API const char* VB_CALL VB_LLM_QuerySimple(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt) {
	if (!a_engine_handle || !a_prompt) {
		printf("[VoxBox LLM] Invalid engine handle and/or missing prompt.\n");
		return nullptr;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	VoxBox::SInferenceResult cpp_result = llm_engine->Query(a_prompt);

	if (!cpp_result.Success() || cpp_result.m_text.empty()) {
		return nullptr;
	}
	return DuplicateString(cpp_result.m_text);
}

VB_LLM_API VB_LLM_EmbeddingResult_t VB_CALL VB_LLM_CreateEmbeddings(VB_LLM_EngineHandle_t a_engine_handle, const char* a_prompt) {
	VB_LLM_EmbeddingResult_t result = { nullptr, 0, 0 };
	
	if (!a_engine_handle || !a_prompt) {
		printf("[VoxBox LLM] Invalid engine handle and/or missing prompt.\n");
		return result;
	}
	
	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	VoxBox::SEmbeddingResult cpp_result = llm_engine->CreateEmbeddings(a_prompt);
	
	if (cpp_result.Success() && !cpp_result.m_embeddings.empty()) {
		size_t byte_count = cpp_result.m_embeddings.size() * sizeof(float);
		result.m_embeddings = static_cast<float*>(malloc(byte_count));
		
		if (result.m_embeddings) {
			std::memcpy(result.m_embeddings, cpp_result.m_embeddings.data(), byte_count);
			result.m_count		= static_cast<int>(cpp_result.m_embeddings.size());
			result.m_success	= 1;
		}
	}
	return result;
}

VB_LLM_API int VB_CALL VB_LLM_GetTokenCount(VB_LLM_EngineHandle_t a_engine_handle, const char* a_text, int a_add_special) {
	if (!a_engine_handle || !a_text) {
		printf("[VoxBox LLM] Invalid engine handle and/or text.\n");
		return -1;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->GetTokenCount(a_text, a_add_special != 0);
}

VB_LLM_API void VB_CALL VB_LLM_Reset(VB_LLM_EngineHandle_t a_engine_handle, const char* a_sys_prompt) {
	// NOTE: Nullptr sys prompt is fine.
	// it just means to not change the sys prompt 
	// when calling the core engine function
	
	if (!a_engine_handle) { 
		printf("[VoxBox LLM] Invalid engine handle.\n");
		return;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	llm_engine->Reset(a_sys_prompt); 
}

VB_LLM_API void VB_CALL VB_LLM_SetTokenCallback(VB_LLM_EngineHandle_t a_engine_handle, VB_LLM_TokenCallbackFn a_callback) {
	if(!a_engine_handle) {
		printf("[VoxBox LLM] Invalid engine handle.\n");
		return;
	}
	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	
	if(!a_callback) {
		printf("[VoxBox LLM] Invalid callback function. Setting to internal callback to nullptr and then returning...\n");
		llm_engine->SetTokenCallback(nullptr);
		return;
	}

	// NOTE:
	// C callback return:   int,  non-zero = interrupt
	// C++ callback return: bool, true = interrupt

	llm_engine->SetTokenCallback(
		[a_callback](int a_token_id, const char* a_token_text, int a_token_type, const float* a_probabilities) -> bool {
			return a_callback(a_token_id, a_token_text, a_token_type, a_probabilities) != 0;
		}
	); // TODO: Probably make a macro for this (i.e., VB_BIND_TOKEN_CALLBACK())

}

VB_LLM_API void VB_CALL VB_LLM_SnapshotClear(VB_LLM_EngineHandle_t a_engine_handle) {
	if (!a_engine_handle) {
		printf("[VoxBox LLM] Invalid engine handle.\n");
		return;
	}
	
	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	llm_engine->SnapshotClear();
}

VB_LLM_API int VB_CALL VB_LLM_SnapshotUpdate(VB_LLM_EngineHandle_t a_engine_handle) {
	if (!a_engine_handle) {
		printf("[VoxBox LLM] Invalid engine handle.\n");
		return 0;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->SnapshotUpdate() ? 1 : 0;
}

VB_LLM_API int VB_CALL VB_LLM_SnapshotRestore(VB_LLM_EngineHandle_t a_engine_handle) {
	if (!a_engine_handle) {
		printf("[VoxBox LLM] Invalid engine handle.\n");
		return 0;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->SnapshotRestore() ? 1 : 0;
}

VB_LLM_API int VB_CALL VB_LLM_SnapshotToFile(VB_LLM_EngineHandle_t a_engine_handle, const char* a_file_path) {
	if (!a_engine_handle || !a_file_path) {
		printf("[VoxBox LLM] Invalid engine handle and/or file path.\n");
		return 0;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->SnapshotToFile(a_file_path) ? 1 : 0;
}

VB_LLM_API int VB_CALL VB_LLM_SnapshotFromFile(VB_LLM_EngineHandle_t a_engine_handle, const char* a_file_path) {
	if (!a_engine_handle || !a_file_path) {
		printf("[VoxBox LLM] Invalid engine handle and/or file path.\n");
		return 0;
	}

	auto* llm_engine = reinterpret_cast<VoxBox::CLLMEngineImpl*>(a_engine_handle);
	return llm_engine->SnapshotFromFile(a_file_path) ? 1 : 0;
}

// TODO: This may not even be needed in all honesty
VB_LLM_API void VB_CALL VB_LLM_FreeString(const char* a_string) {
	if (a_string) {
		free(const_cast<char*>(a_string));
	}
}

VB_LLM_API void VB_CALL VB_LLM_FreeFloatArray(float* a_float_array) {
	if (a_float_array) {
		free(a_float_array);
	}
}

VB_LLM_API void VB_CALL VB_LLM_FreeInferenceResult(VB_LLM_InferenceResult_t* a_inference_result) {
	if (a_inference_result) {
		if (a_inference_result->m_text) {
			free(a_inference_result->m_text);
			a_inference_result->m_text = nullptr;
		}
		a_inference_result->m_success = 0;
	}
}

VB_LLM_API void VB_CALL VB_LLM_FreeEmbeddingResult(VB_LLM_EmbeddingResult_t* a_embedding_result) {
	if (a_embedding_result) {
		if (a_embedding_result->m_embeddings) {
			free(a_embedding_result->m_embeddings);
			a_embedding_result->m_embeddings = nullptr;
		}
		a_embedding_result->m_count = 0;
		a_embedding_result->m_success = 0;
	}
}

VB_LLM_API VB_LLM_Config_t VB_CALL VB_LLM_GetDefaultConfig() {
	VB_LLM_Config_t config = {};

	config.m_model_file_path		= nullptr;
	config.m_grammar				= nullptr;

	config.m_sys_prompt				= nullptr;
	config.m_sys_prompt_beg_delim	= nullptr;
	config.m_sys_prompt_mid_delim	= nullptr;
	config.m_sys_prompt_end_delim	= nullptr;
	config.m_prompt_beg_delim		= nullptr;
	config.m_prompt_end_delim		= nullptr;
	config.m_think_beg_delim		= nullptr;
	config.m_think_end_delim		= nullptr;

	config.m_n_ctx			= 0;	// Vocab size
	config.m_n_threads		= -1;	// Auto-detect
	config.m_seed			= -1;	// Random
	config.m_n_gpu_layers	= -1;	// Offload all

	config.m_top_k			= 0;	
	config.m_top_p			= 1.0f;	// Disabled
	config.m_min_p			= 0.0f;	// Disabled
	config.m_temp			= 0.0f;	// Greedy sampling + no probability output

	config.m_try_prompts_by_model	= 0;
	config.m_embeddings				= 0;

	return config;
}

VB_LLM_API const char* VB_CALL VB_LLM_GetVersion() {
	return VB_LLM_VERSION;
}

#ifdef __cplusplus
namespace VoxBox {
	CLLMEngine::CLLMEngine(const SLLMConfig& a_config) {
		m_llm_engine = std::make_unique<CLLMEngineImpl>(a_config);
	}

	CLLMEngine::~CLLMEngine() = default; // Defined here for visibility

	CLLMEngine::CLLMEngine(CLLMEngine&& a_other) noexcept 
		: m_llm_engine(std::move(a_other.m_llm_engine)) {
	}

	void CLLMEngine::operator=(CLLMEngine&& a_other) noexcept {
		m_llm_engine = std::move(a_other.m_llm_engine);
	}

	bool CLLMEngine::IsInitialized() const {
		return m_llm_engine ? m_llm_engine->IsInitialized() : false;
	}

	SInferenceResult CLLMEngine::Query(const std::string& a_prompt, bool a_skip_sys_prompt_end, bool a_resume_from_snapshot) {
		return (!m_llm_engine) ? SInferenceResult{ "", EResultCode::NotInitialized } : m_llm_engine->Query(a_prompt.c_str(), a_skip_sys_prompt_end, a_resume_from_snapshot);
	}

	SEmbeddingResult CLLMEngine::CreateEmbeddings(const std::string& a_prompt) {
		return (!m_llm_engine) ? SEmbeddingResult{ {}, EResultCode::NotInitialized } : m_llm_engine->CreateEmbeddings(a_prompt.c_str());
	}

	int CLLMEngine::GetTokenCount(const std::string& a_text, bool a_add_special_char) {
		return (!m_llm_engine) ? -1 : m_llm_engine->GetTokenCount(a_text.c_str(), a_add_special_char);
	}

	void CLLMEngine::SetTokenCallback(TokenCallbackFn a_callback) {
		if (m_llm_engine) {
			m_llm_engine->SetTokenCallback(std::move(a_callback));
		}
	}

	void CLLMEngine::SetSamplerConfig(const SLLMSamplerConfig& a_config) {
		if (m_llm_engine) {
			m_llm_engine->SetSamplerConfig(a_config);
		}
	}

	void CLLMEngine::Reinit(const SLLMConfig& a_config) {
		if (m_llm_engine) {
			m_llm_engine->Reinit(a_config);
		}
	}

	void CLLMEngine::Reset(const std::string& a_sys_prompt) {
		if (m_llm_engine) {
			m_llm_engine->Reset(a_sys_prompt.empty() ? nullptr : a_sys_prompt.c_str());
		}
	}

	void CLLMEngine::SnapshotClear() {
		if (m_llm_engine) {
			m_llm_engine->SnapshotClear();
		}
	}

	bool CLLMEngine::SnapshotUpdate() {
		return m_llm_engine && m_llm_engine->SnapshotUpdate();
	}

	bool CLLMEngine::SnapshotRestore() {
		return m_llm_engine && m_llm_engine->SnapshotRestore();
	}

	bool CLLMEngine::SnapshotToFile(const std::string& a_file_path) {
		return m_llm_engine && m_llm_engine->SnapshotToFile(a_file_path.c_str());
	}

	bool CLLMEngine::SnapshotFromFile(const std::string& a_file_path) {
		return m_llm_engine && m_llm_engine->SnapshotFromFile(a_file_path.c_str());
	}

	std::unique_ptr<CLLMEngine> CLLMEngine::Create(const SLLMConfig& a_config) {
		auto engine = std::make_unique<CLLMEngine>(a_config);
		if (!engine->IsInitialized()) {
			return nullptr;
		}
		return engine;
	}

	SLLMConfig CLLMEngine::GetDefaultConfig() {
		return SLLMConfig();
	}

	const char* CLLMEngine::GetVersion() {
		return VB_LLM_VERSION;
	}
}
#endif