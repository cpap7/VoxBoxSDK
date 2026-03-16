#include "vbppch.h"
#include "TTSEngine.h"


namespace VoxBox {
	CTTSEngineImpl::CTTSEngineImpl(const STTSConfig& a_piper_config)
		: m_config(a_piper_config), m_audio_stream_buffer(std::make_unique<CTTSAudioStreamBuffer>()) {

		Init(a_piper_config);
	}

	CTTSEngineImpl::~CTTSEngineImpl() {
		Shutdown();
	}

	void CTTSEngineImpl::Init(const STTSConfig& a_config) {
		if (m_is_initialized) {
			Shutdown();
		}
		m_config = a_config; // Update config here in case of reinit
		
		m_piper_config = new piper::PiperConfig();
		m_piper_voice = new piper::Voice();

		LoadVoice();
		ApplySynthesisConfig();

		piper::initialize(*m_piper_config);

		m_audio_stream_buffer->SetSampleRate(m_piper_voice->synthesisConfig.sampleRate); // 22050 Hz
		
		m_is_initialized = true;
	}

	void CTTSEngineImpl::Reinit(const STTSConfig& a_config) {
		Shutdown();
		Init(a_config);
	}

	void CTTSEngineImpl::Shutdown() {
		if (!m_is_initialized) {
			return;
		}

		piper::terminate(*m_piper_config);
		
		delete m_piper_config;
		m_piper_config = nullptr;

		delete m_piper_voice;
		m_piper_voice = nullptr;

		m_audio_stream_buffer->Reset();
		
		m_is_initialized = false;
	}

	std::vector<int16_t> CTTSEngineImpl::Synthesize(const char* a_text) {
		assert(m_is_initialized);
		assert(a_text != nullptr);

		std::vector<int16_t> audio_buffer;
		piper::SynthesisResult result;
		
		piper::textToAudio(
			*m_piper_config,
			*m_piper_voice,
			a_text,
			audio_buffer,
			result,
			nullptr
		);

		return audio_buffer;
	}

	void CTTSEngineImpl::SynthesizeToWAVFile(const char* a_text, const char* a_wav_path) {
		assert(m_is_initialized);
		assert(a_text != nullptr);
		assert(a_wav_path != nullptr);

		piper::SynthesisResult result;
		std::ofstream audio_file(a_wav_path, std::ios::binary);

		piper::textToWavFile(
			*m_piper_config,
			*m_piper_voice,
			a_text,
			audio_file,
			result
		);
	}

	void CTTSEngineImpl::SynthesizeStreaming(std::istream& a_input, std::ostream& a_output) {
		assert(m_is_initialized);

		std::string line;

		while (std::getline(a_input, line)) {
			piper::SynthesisResult result;

			std::vector<int16_t> audio_buffer;
			m_audio_stream_buffer->Reset();

			// Output thread will consume audio & write to stream
			std::thread output_thread([this, &a_output]() {
				std::vector<int16_t> samples;

				while (m_audio_stream_buffer->WaitAndConsume(samples)) {
					a_output.write(
						reinterpret_cast<const char*>(samples.data()),
						sizeof(int16_t) * samples.size()
					);

					a_output.flush();
					samples.clear();
				}
			});

			auto AudioCallbackFn = [this, &audio_buffer]() {
				m_audio_stream_buffer->Push(audio_buffer);
			};

			piper::textToAudio(*m_piper_config, *m_piper_voice, line, audio_buffer, result, AudioCallbackFn);
			
			m_audio_stream_buffer->MarkAsFinished();
			output_thread.join();
		}
	}

	void CTTSEngineImpl::SetSynthesisConfig(const SSynthesisConfig& a_synthesis_config) {
		m_config.m_synthesis_config = a_synthesis_config;

		if (m_is_initialized) {
			ApplySynthesisConfig();
		}
	}

	void CTTSEngineImpl::LoadVoice() {
		bool use_cuda = (m_config.m_hardware_config.m_gpu_acceleration_type == EGPUBackendType::Cuda);
		
		piper::loadVoice(
			*m_piper_config,
			m_config.m_voice_config.m_model_onnx_path.string(),
			m_config.m_voice_config.m_model_onnx_json_path.string(),
			*m_piper_voice,
			m_speaker_id,
			use_cuda		
		);

		// Configure espeak-ng data path
		if (m_piper_voice->phonemizeConfig.phonemeType == piper::eSpeakPhonemes) {
			if (m_config.m_voice_config.m_espeak_data_path != "") {
				// Set to user provided path
				m_piper_config->eSpeakDataPath = m_config.m_voice_config.m_espeak_data_path.string();
			}
			else { // Assume it's next to the executable binary file
				m_piper_config->eSpeakDataPath = std::filesystem::absolute(GetExecutablePath().parent_path() / "espeak-ng-data").string();
			}
		}
		else {
			m_piper_config->useESpeak = false;
		}
		
		// Configure libtashkeel for Arabic
		if(m_piper_voice->phonemizeConfig.eSpeak.voice == "ar") {
			m_piper_config->useTashkeel = true;

			if (m_config.m_voice_config.m_tashkeel_model_path) {
				// Set to user provided path
				m_piper_config->tashkeelModelPath = m_config.m_voice_config.m_tashkeel_model_path.value().string();
			}
			else { // Assume it's next to the executable binary file
				m_piper_config->tashkeelModelPath = std::filesystem::absolute(GetExecutablePath().parent_path() / "libtashkeel_model.ort").string();
			}
		}
	}

	void CTTSEngineImpl::ApplySynthesisConfig() {
		auto& piper_synthesis_config = m_piper_voice->synthesisConfig;
		const auto& vb_synthesis_config = m_config.m_synthesis_config;

		// Adjust piper config values to user-specified values if available
		if (vb_synthesis_config.m_noise_scale) {
			piper_synthesis_config.noiseScale = vb_synthesis_config.m_noise_scale.value();
		}
		if (vb_synthesis_config.m_length_scale) {
			piper_synthesis_config.lengthScale = vb_synthesis_config.m_length_scale.value();
		}
		if (vb_synthesis_config.m_noise_width) {
			piper_synthesis_config.noiseW = vb_synthesis_config.m_noise_width.value();
		}
		if (vb_synthesis_config.m_sentence_silence_seconds) {
			piper_synthesis_config.sentenceSilenceSeconds = vb_synthesis_config.m_sentence_silence_seconds.value();
		}

		if (m_phoneme_silence_seconds) {
			if (!piper_synthesis_config.phonemeSilenceSeconds) {
				// Overwrite
				piper_synthesis_config.phonemeSilenceSeconds = m_phoneme_silence_seconds;
			}
			else {
				// Merge
				for (const auto& [phoneme, silence_seconds] : *m_phoneme_silence_seconds) {
					piper_synthesis_config.phonemeSilenceSeconds->try_emplace(phoneme, silence_seconds);
				}
			}
		}
	}

	std::filesystem::path CTTSEngineImpl::GetExecutablePath() {
#ifdef _MSC_VER // Windows
		wchar_t module_path[MAX_PATH] = { 0 };
		GetModuleFileNameW(nullptr, module_path, std::size(module_path));
		return std::filesystem::path(module_path);

#elif defined (__APPLE__) // MacOSX
		char module_path[PATH_MAX] = { 0 };
		uint32_t size = std::size(module_path);
		_NSGetExecutablePath(module_path, &size);
		return std::filesystem::path(module_path);

#else // Linux
		return std::filesystem::canonical("/proc/self/exe");
#endif

	}
}