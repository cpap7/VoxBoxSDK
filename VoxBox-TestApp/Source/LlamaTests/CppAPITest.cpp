#include <iostream>

#include <VoxBoxLLM.h>

static bool TokenCallback(int a_token_id, const char* a_token_text, int a_token_type, const float* a_probabilities) {
	// Only print sampled non-EOG (end-of-generation) tokens
	// AKA actual response text
	if (a_token_type == static_cast<int>(VoxBox::ETokenType::SampledNonEOGNonCtrl)) {
		std::cout << a_token_text << std::flush;
	}
	return false; // false = continue, true = interrupt
}

int main() {
	// Setup config
	const char* model_path = "Assets/Models/llama-3.2-1b-instruct-q8_0.gguf";
	VoxBox::SLLMConfig config = VoxBox::CLLMEngine::GetDefaultConfig();
	config.m_model_config.m_model_file_path			= model_path;
	config.m_model_config.m_n_gpu_layers			= 99;
	config.m_prompt_config.m_sys_prompt				= "You are a helpful AI assistant.";
	config.m_prompt_config.m_try_prompts_by_model	= true;
	config.m_token_callback_function				= TokenCallback;

	// Setup engine
	auto engine = VoxBox::CLLMEngine::Create(config);
	if (!engine) {
		std::cerr << "[VoxBox LLM] Failed to create engine!\n";
		return 1;
	}

	std::cout << "[VoxBox LLM] Engine initialized. Version: " << VoxBox::CLLMEngine::GetVersion() << "\n";

	// Basic inference test
	std::cout << "\n- BASIC QUERY -\n";
	const char* basic_query_test = "What's 2 + 2?";
	std::cout << "[VoxBox LLM] Query: " << basic_query_test << "\n";
	VoxBox::SInferenceResult result = engine->Query(basic_query_test);
	if (result.Success()) {
		std::cout << "[VoxBox LLM] Basic query test complete!\n";
	}

	// Token count test
	const char* token_count_test = "Hello, world!";
	int token_count = engine->GetTokenCount(token_count_test);
	std::cout << "\n[VoxBox LLM] Token count for '" << token_count_test << "': " << token_count << "\n";

	// Snapshot tests
	std::cout << "\n - SNAPSHOT TEST -\n";
	std::cout << "[VoxBox LLM] Querying w/ skip_sys_prompt_end...\n";
	const char* snapshot_query = "Tell me a short joke";
	std::cout << "[VoxBox LLM] Query input: " << snapshot_query << "\n";
	engine->Query(snapshot_query, true); // Don't run inference yet, just decode the prompt

	if (engine->SnapshotUpdate()) {
		std::cout << "[VoxBox LLM] Snapshot captured\n";

		// Save to file, then reload from file
		engine->SnapshotToFile("test_snapshot.bin");
		std::cout << "[VoxBox LLM] Snapshot saved\n";

		engine->SnapshotClear();
		std::cout << "[VoxBox LLM] Snapshot cleared\n";

		engine->SnapshotFromFile("test_snapshot.bin");
		std::cout << "[VoxBox LLM] Snapshot loaded from file\n";

		// Restore & resume inference from snapshot
		if (engine->SnapshotRestore()) {
			std::cout << "[VoxBox LLM] Snapshot restored. Resuming inference...\n";
			VoxBox::SInferenceResult snapshot_result = engine->Query(snapshot_query, false, true);
			if (snapshot_result.Success()) {
				std::cout << "\n[VoxBox LLM] Snapshot test complete!\n";
			}
		}
	}

	// Multi-turn conversation test
	std::cout << "\n- MULTI-TURN CONVO TEST -\n";
	const char* multi_turn_query_test = "My name is Billy-bob";
	std::cout << "\n[VoxBox LLM] Query input: " << multi_turn_query_test << "\n";
	engine->Reset("");
	engine->Query(multi_turn_query_test);
	VoxBox::SInferenceResult follow_up = engine->Query("Tell me, what's my name?");
	if (follow_up.Success()) {
		std::cout << "\n[VoxBox LLM] Multi-turn conversation test complete!\n";
	}

	// Sampler config change test
	std::cout << "\n- SAMPLER CONFIG TEST -\n";
	VoxBox::SLLMSamplerConfig sampler_cfg;
	sampler_cfg.m_temp = 0.8f;
	sampler_cfg.m_top_p = 0.9f;
	sampler_cfg.m_top_k = 40;
	engine->SetSamplerConfig(sampler_cfg);

	engine->Reset("");
	const char* creative_query_test = "Write a haiku about how much you hate sea turtles.";
	std::cout << "\n[VoxBox LLM] Query input: " << creative_query_test << "\n";

	VoxBox::SInferenceResult creative_response_result = engine->Query(creative_query_test);
	if (creative_response_result.Success()) {
		std::cout << "\n[VoxBox LLM] Creative query test complete!\n";
	}

	std::cout << "[VoxBox LLM] Tests complete\n";
	return 0;
}