#pragma once
#include <cstdint>
#include <string>

namespace VoxBox {
	
	enum class VB_STT_API EHardwareBackendType : uint8_t {
		Auto	= 0, // whisper.cpp automatically decides

		// Hardware
		CPU		= 1, // Force CPU only
		Vulkan	= 2, // Request Vulkan support
		CUDA	= 3, // Request CUDA support
		
		Default = Auto
	};
	
	struct SHardwareConfig {
		int m_threads					= 0;		// Default = 0 --> whisper.cpp auto detects # of threads to use
		EHardwareBackendType m_backend	= EHardwareBackendType::Auto;
		bool m_use_gpu					= false;	// (m_backend == EHardwareBackendType::CUDA || m_backend == EHardwareBackendType::Vulkan) ? true : false;
		//bool m_flash_attention		= false;
	};

	// Used for acceleration backend detection at runtime via whisper methods
	struct SSystemConfig {
		std::string m_system_info; // Via whisper_print_info()

		/* Flags */
		// CPU
		bool m_has_noavx	= false;
		bool m_has_avx		= false;
		bool m_has_avx2		= false;
		bool m_has_avx512	= false;

		// GPU
		bool m_has_vulkan	= false;
		bool m_has_cuda		= false;
	};
	
	// OpenVINO acceleration (if compiled with support)
	struct SOpenVINOConfig {
		std::string m_encode_device = "CPU";  // OpenVINO encoder device
	};
}