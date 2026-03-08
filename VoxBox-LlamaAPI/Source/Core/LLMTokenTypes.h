#pragma once

#include <cstdint>
#include <string>

namespace VoxBox {
	enum class ETokenType : uint8_t {
		Unknown = 0,
	
		Prompt,					// Tokens represent a user prompt
		InterruptRequest,		// Tokens to-be-added on (user) interrupt request to stop inference
		SampledNonEOGNonCtrl,	// Thinking mode tokens & their begin/end tags + delimiters have their own token type
		SampledEOG,				// An EOG (end-of-generation) token, e.g. EOS or EOT
		Delimiter,				// For prompt delimiters, used by/for instruction-tuned models
		SystemPrompt,			// Tokens of the actual system prompt (text)
		SampledCtrlNonEOG,		// Non-EOG control tokens
		ThinkBegin,				// Token that represents the thinking begin delimiter
		ThinkText,				// Tokens that represent the thinking mode text
		ThinkEnd				// Token that represents the thinking end delimiter
	};

	static std::string ConvertTokenTypeToString(ETokenType a_token_type) {
		std::string token_type;
		switch (a_token_type) {
			case ETokenType::Unknown:				token_type = "UNKNOWN";						break;
			case ETokenType::Prompt:				token_type = "PROMPT";						break;
			case ETokenType::InterruptRequest:		token_type = "INTERRUPT REQUEST";			break;
			case ETokenType::SampledNonEOGNonCtrl:  token_type = "SAMPLED NON-EOG NON-CTRL";	break;
			case ETokenType::SampledEOG:			token_type = "SAMPLED EOG";					break;
			case ETokenType::Delimiter:				token_type = "DELIMITER";					break;
			case ETokenType::SystemPrompt:			token_type = "SYSTEM PROMPT";				break;
			case ETokenType::SampledCtrlNonEOG:		token_type = "SAMPLED CTRL NON-EOG";		break;
			case ETokenType::ThinkBegin:			token_type = "THINK BEGIN";					break;
			case ETokenType::ThinkText:				token_type = "THINK TEXT";					break;
			case ETokenType::ThinkEnd:				token_type = "THINK END";					break;

		}

		return token_type;
	}
}
