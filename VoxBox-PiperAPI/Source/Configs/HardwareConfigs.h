#pragma once

namespace VoxBox {

    // Note: Could possibly use a boolean
    enum class EGPUBackendType : uint8_t {
        None    = 0,

        // GPU acceleration options
        // TODO: Investigate other GPU acceleration options included for Piper
        Cuda    = 1,    

        Default = None
    };
    
    
    struct SHardwareConfig {
        EGPUBackendType m_gpu_acceleration_type = EGPUBackendType::Default;
    };
}