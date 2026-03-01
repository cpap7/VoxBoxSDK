#pragma once

#include "TTSConfig.h"
#include "TTSAudioStreamBuffer.h"
#include <optional>

#include <piper.hpp>

namespace VoxBox {
	// TTS Engine

	class CTTSEngineImpl {
	private:
		// VoxBox-defined config struct for piper
		STTSConfig m_config; // Cached for reinitialization

		// Speech-synthesis state
		std::optional<std::map<piper::Phoneme, float>> m_phoneme_silence_seconds = std::nullopt;
		std::optional<piper::SpeakerId> m_speaker_id = std::nullopt;

		// Piper-defined data type ptrs
		piper::PiperConfig* m_piper_config = nullptr;
		piper::Voice* m_piper_voice = nullptr;

		// Audio output
		std::unique_ptr<CCoreTTSAudioStreamBuffer> m_audio_stream_buffer = nullptr;
		int m_sample_rate = 22050;
		
		// Flags
		bool m_is_initialized = false; // Tracks state

	public:
		//CTTSEngineImpl() = delete;
		//CTTSEngineImpl(const CTTSEngineImpl&) = delete;
		//void operator=(const CTTSEngineImpl&) = delete;

		explicit CTTSEngineImpl(const STTSConfig& a_piper_config);
		~CTTSEngineImpl();
		
		// Lifecycle
		void Init(const STTSConfig& a_config);
		void Reinit(const STTSConfig& a_config);
		void Shutdown();

		// Speech synthesis-related
		std::vector<int16_t> Synthesize(const char* a_text);
		void SynthesizeToWAVFile(const char* a_text, const char* a_wav_path);
		void SynthesizeStreaming(std::istream& a_input, std::ostream& a_output);
		
		// Getters
		inline bool IsInitialized() const								{ return m_is_initialized;				}
		inline int GetSampleRate() const								{ return m_sample_rate;					}
		inline CCoreTTSAudioStreamBuffer* GetAudioStreamBuffer() const  { return m_audio_stream_buffer.get();	}
		inline const STTSConfig& GetConfig() const						{ return m_config;						}
		inline const SSynthesisConfig& GetSynthesisConfig() const		{ return m_config.m_synthesis_config;	}
		//inline SSynthesisConfig GetDefaultSynthesisConfig()			{ return SSynthesisConfig();			}

		// Setters (runtime adjustment)
		void SetSynthesisConfig(const SSynthesisConfig& a_synthesis_config);
		inline void SetSpeakerID(piper::SpeakerId a_id) { m_speaker_id = a_id; } // piper::SpeakerId passed by value since it's technically int64_t

		

	private:
		// Internal helpers
		void LoadVoice();
		void ApplySynthesisConfig();
		static std::filesystem::path GetExecutablePath();
	};

}
