-- premake5.lua
project "VoxBox-WhisperAPI"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    -- Precompiled header
    pchheader "vbwpch.h"
    pchsource "Source/vbwpch.cpp"

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "Source",                                    -- For vbwpch.h
        "../VoxBox-Shared",                          -- For useful, global utilities like PlatformDetection.h
        "Vendor/whisper.cpp/include",                -- whisper.h
        "Vendor/whisper.cpp/ggml/include",           -- ggml headers
    }

    libdirs {
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/src/%{cfg.buildcfg}",                      -- whisper.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/%{cfg.buildcfg}",                 -- ggml.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-base/%{cfg.buildcfg}",       -- ggml-base.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-cpu/%{cfg.buildcfg}",        -- ggml-cpu.lib
    }

    links {
        "whisper",
        "ggml",
        "ggml-base",
        "ggml-cpu",
    }

    filter "system:windows"
        systemversion "latest"
        characterset "Unicode"
        defines { 
            "_WINDOWS",
            "_USRDLL",
            "VB_EXPORT_STT" -- Export dll symbols
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }