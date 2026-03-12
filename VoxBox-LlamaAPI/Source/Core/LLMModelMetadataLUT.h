#pragma once
//#include "LLMConfig.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace VoxBox {
	
	// Model names

	static inline std::vector<std::string> s_qwen_model_names = {
		// Model names
		"Qwen2-0.5B-Instruct",
		"Qwen2-1.5B-Instruct",
		"qwen2-1_5b-instruct",
		"Qwen2.5 0.5B Instruct",
		"Qwen2.5 1.5B Instruct",
		"Qwen2.5 3B Instruct",
		"Qwen2.5 7B Instruct",
		"Qwen3-4B-Instruct-2507",
		"Qwen3-Vl-8B-Instruct",
		"Qwen3-30B-A3B-Instruct-2507",
		"Qwen3-Next-80B-A3B-Instruct",
		"Virtuoso Small v2",
		"Finalize Slerp",
		"MiniCPM3 4B",
		"SmolLM 135M",
		"SmolLM2 135M Instruct",
		"SmolLM2 360M Instruct",
		"SmolLM2 1.7B Instruct"
	};

	static inline std::vector<std::string> s_qwen3_model_names = {
		"Qwen3-0.6B",
		"Qwen3-1.7B",
		"Qwen3 8B",
		"Qwen3 14B",
		"Qwen3-14B",
		"Qwen3-30B-A3B",
		"Qwen3-32B",
		"Qwen3.5-35B-A3B",
		"Qwen3.5-4B",
		"Qwen3.5-9B",
		"Qwen3.5-27B",
		"SmolLM3 3B", // Officially recommended: Temperature = 0.6, top-p = 0.95.
		// Add more, when necessary.
	};

	static inline std::vector<std::string> s_phi3_model_names = {
		"Phi 3",
		"Phi 3 Mini 4k Instruct",
		"Phi 3.5 Mini Instruct"
	};

	static inline std::vector<std::string> s_phi4_model_names = {
		"Phi 4"
	};

	static inline std::vector<std::string> s_llama2_model_names = {
		"LLaMA v2"
	};

	static inline std::vector<std::string> s_llama3_model_names = {
		"Meta-Llama-3-8B-Instruct", 
		"Meta Llama 3 8B Instruct",
		"Llama 3.1 SauerkrautLM 8b Instruct", 
		"Meta Llama 3.1 8B Instruct",
		"Llama 3.2 3B Instruct", 
		"Llama 3.2 1B Instruct", 
		"Llama-3.3-70B-Instruct"
	};

	static inline std::vector<std::string> s_nemo3nano_model_names = {
		"Nemotron-3-Nano-30B-A3B"
	};

	static inline std::vector<std::string> s_gemma_model_names = {
		"Gemma-3-270M-It", 
		"Gemma 2 2b It", 
		"Gemma 3 1b It", 
		"Gemma-3-1B-It",
		"Gemma 3 4b It", 
		"Gemma-3-4B-It", 
		"Gemma 3 12b It", 
		"Gemma-3-12B-It",
		"Gemma-3-27B-It", 
		"Gemma-3N-E2B-It", 
		"Gemma-3N-E4B-It"
	};

	static inline std::vector<std::string> s_exaone3_model_names = {
		"EXAONE 3.0 7.8B Instruct"
	};

	static inline std::vector<std::string> s_cohere_model_names = {
		"CohereForAI.aya Expanse 8b", 
		"Aya Expanse 8b", 
		"C4Ai Command R7B 12 2024"
	};

	static inline std::vector<std::string> s_mistral_model_names = {
		"Mistral Small 24B Instruct 2501", 
		"Mistral-Small-3.2-24B-Instruct-2506",
		"Ministral-3-8B-Instruct-2512"
	};

	static inline std::vector<std::string> s_mistral7b_v02_model_names = {
		"mistralai_mistral-7b-instruct-v0.2"
	};

	static inline std::vector<std::string> s_olmo_model_names = {
		"OLMo 2 0425 1B Instruct"
	};

	static inline std::vector<std::string> s_granite4_model_names = {
		"Granite-4.0-H-1B", 
		"Granite-4.0-H-Micro"
	};

	static inline std::vector<std::string> s_ernie45moe_model_names = {
		"Ernie-4.5-21B-A3B-Pt"
	};

	static inline std::vector<std::string> s_glm_model_names = {
		"Glm-4.7-Flash", 
		"cerebras/GLM-4.7-Flash-REAP-23B-A3B"
	};


	// Prompt templates
	struct SLLMPromptTemplate {
		//std::string m_sys_prompt;				// The actual system prompt text
		std::string m_sys_prompt_beg_delim;		// e.g. "<|im_start|>system\n"
		std::string m_sys_prompt_mid_delim;		// e.g. "<|im_end|>\n<|im_start|>user\n"
		std::string m_sys_prompt_end_delim;		// e.g. "<|im_end|>\n<|im_start|>assistant\n"
		std::string m_prompt_beg_delim;			// e.g. "<|im_start|>user\n"
		std::string m_prompt_end_delim;			// e.g. "<|im_end|>\n<|im_start|>assistant\n"
		std::string m_think_beg_delim;			// e.g. "<think>"
		std::string m_think_end_delim;			// e.g. "</think>"
	};

	// ChatML (Qwen2, Qwen2.5, SmolLM, MiniCPM, etc.)
	static inline SLLMPromptTemplate s_qwen_model_template = {
		"<|im_start|>system\n",
		"<|im_end|>\n<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n",
		"<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n",
		"", ""
	};

	// Qwen3 (ChatML + thinking disabled via empty think block in delimiters)
	static inline SLLMPromptTemplate s_qwen3_model_template = {
		"<|im_start|>system\n",
		"<|im_end|>\n<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n<think>\n\n</think>\n\n",
		"<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n<think>\n\n</think>\n\n",
		"<think>", "</think>"
	};

	static inline SLLMPromptTemplate s_phi3_model_template = {
		"<|system|>\n",
		"<|end|>\n<|user|>\n",
		"<|end|>\n<|assistant|>\n",
		"\n<|user|>\n",
		"<|end|>\n<|assistant|>\n",
		"", ""
	};

	static inline SLLMPromptTemplate s_phi4_model_template = {
		"<|im_start|>system<|im_sep|>",
		"<|im_end|><|im_start|>user<|im_sep|>",
		"<|im_end|><|im_start|>assistant<|im_sep|>",
		"<|im_start|>user<|im_sep|>",
		"<|im_end|><|im_start|>assistant<|im_sep|>",
		"", ""
	};

	static inline SLLMPromptTemplate s_llama2_model_template = {
		"[INST] <<SYS>>\n",
		"\n<</SYS>>\n\n",
		" [/INST] ",
		"\n<s>[INST] ",
		" [/INST]",
		"", ""
	};

	static inline SLLMPromptTemplate s_llama3_model_template = {
		"<|start_header_id|>system<|end_header_id|>\n\n",
		"<|eot_id|><|start_header_id|>user<|end_header_id|>\n\n",
		"<|eot_id|><|start_header_id|>assistant<|end_header_id|>",
		"<|start_header_id|>user<|end_header_id|>\n\n",
		"<|eot_id|><|start_header_id|>assistant<|end_header_id|>",
		"", ""
	};

	// Nemotron-3-Nano (thinking disabled for 1st answer, enabled for follow-ups)
	static inline SLLMPromptTemplate s_nemo3nano_model_template = {
		"<|im_start|>system\n",
		"<|im_end|>\n<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n<think></think>",
		"<|im_start|>user\n",
		"<|im_end|>\n<|im_start|>assistant\n<think>\n",
		"<think>", "</think>"
	};

	// Gemma (system prompt piggybacks on first user turn)
	static inline SLLMPromptTemplate s_gemma_model_template = {
		"<start_of_turn>user\n",
		"\n\n",
		"<end_of_turn>\n<start_of_turn>model\n",
		"<start_of_turn>user\n",
		"<end_of_turn>\n<start_of_turn>model\n",
		"", ""
	};

	static inline SLLMPromptTemplate s_exaone3_model_template = {
		"[|system|]",
		"[|endofturn|]\n[|user|]",
		"\n[|assistant|]",
		"[|user|]",
		"\n[|assistant|]",
		"", ""
	};

	static inline SLLMPromptTemplate s_cohere_model_template = {
		"<|START_OF_TURN_TOKEN|><|SYSTEM_TOKEN|>",
		"<|END_OF_TURN_TOKEN|><|START_OF_TURN_TOKEN|><|USER_TOKEN|>",
		"<|END_OF_TURN_TOKEN|><|START_OF_TURN_TOKEN|><|CHATBOT_TOKEN|>",
		"<|START_OF_TURN_TOKEN|><|USER_TOKEN|>",
		"<|END_OF_TURN_TOKEN|><|START_OF_TURN_TOKEN|><|CHATBOT_TOKEN|>",
		"", ""
	};

	// Mistral V7-Tekken
	static inline SLLMPromptTemplate s_mistral_model_template = {
		"[SYSTEM_PROMPT]",
		"[/SYSTEM_PROMPT][INST]",
		"[/INST]",
		"[INST]",
		"[/INST]",
		"", ""
	};

	// Mistral 7B v0.2 (system prompt prefixed in first user turn)
	static inline SLLMPromptTemplate s_mistral7b_v02_model_template = {
		"[INST]",
		"\n",
		" [/INST]",
		"[INST]",
		" [/INST]",
		"", ""
	};

	static inline SLLMPromptTemplate s_olmo_model_template = {
		"<|system|>\n",
		"\n<|user|>\n",
		"\n<|assistant|>\n",
		"\n<|user|>\n",
		"\n<|assistant|>\n",
		"", ""
	};

	static inline SLLMPromptTemplate s_granite4_model_template = {
		"<|start_of_role|>system<|end_of_role|>",
		"<|end_of_text|>\n<|start_of_role|>user<|end_of_role|>",
		"<|end_of_text|>\n<|start_of_role|>assistant<|end_of_role|>",
		"<|start_of_role|>user<|end_of_role|>",
		"<|end_of_text|>\n<|start_of_role|>assistant<|end_of_role|>",
		"", ""
	};

	static inline SLLMPromptTemplate s_ernie45moe_model_template = {
		"",
		"\nUser: ",
		"\nAssistant: ",
		"<s>User: ",
		"\nAssistant: ",
		"", ""
	};

	// GLM (thinking enabled for follow-ups, disabled for 1st answer)
	static inline SLLMPromptTemplate s_glm_model_template = {
		"[gMASK]<sop><|system|>",
		"<|user|>",
		"<|assistant|></think>",
		"<|user|>",
		"<|assistant|><think>",
		"<think>", "</think>"
	};

	// Lookup model name (from GGUF metadata "general.name") --> prompt template

	struct SEntry {
		const std::vector<std::string>* m_model_names;
		const SLLMPromptTemplate* m_prompt_template;
	};

	static inline const SEntry s_entries[] = {
		{ &s_qwen_model_names,				&s_qwen_model_template			},
		{ &s_qwen3_model_names,				&s_qwen3_model_template			},
		{ &s_phi3_model_names,				&s_phi3_model_template			},
		{ &s_phi4_model_names,				&s_phi4_model_template			},
		{ &s_llama2_model_names,			&s_llama2_model_template		},
		{ &s_llama3_model_names,			&s_llama3_model_template		},
		{ &s_nemo3nano_model_names,			&s_nemo3nano_model_template		},
		{ &s_gemma_model_names,				&s_gemma_model_template			},
		{ &s_exaone3_model_names,			&s_exaone3_model_template		},
		{ &s_cohere_model_names,			&s_cohere_model_template		},
		{ &s_mistral_model_names,			&s_mistral_model_template		},
		{ &s_mistral7b_v02_model_names,		&s_mistral7b_v02_model_template },
		{ &s_olmo_model_names,				&s_olmo_model_template			},
		{ &s_granite4_model_names,			&s_granite4_model_template		},
		{ &s_ernie45moe_model_names,		&s_ernie45moe_model_template	},
		{ &s_glm_model_names,				&s_glm_model_template			}

	};


	inline const std::unordered_map<std::string, SLLMPromptTemplate>& GetModelPromptTemplateMap() {
		// TODO: I don't exactly love the way this is set up, so I'll change this later
		static const auto s_model_map = [] {
			std::unordered_map<std::string, SLLMPromptTemplate> model_template_map;
			for (const auto& entry : s_entries) {
				for (const auto& name : *entry.m_model_names) {
					model_template_map[name] = *entry.m_prompt_template;
				}
			}
			return model_template_map;
		}();

		return s_model_map;
	}


}

