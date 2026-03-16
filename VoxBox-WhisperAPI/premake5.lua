-- premake5.lua
project "VoxBox-WhisperAPI"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off" -- /MD 
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
        "Source",
        "../VoxBox-Common/Source",
        "Vendor/whisper.cpp/include",                -- whisper.h
        "Vendor/whisper.cpp/ggml/include",           -- ggml headers
    }

    libdirs {
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/src/%{cfg.buildcfg}",                      -- whisper.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/%{cfg.buildcfg}",                 -- ggml.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-base/%{cfg.buildcfg}",       -- ggml-base.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-cpu/%{cfg.buildcfg}",        -- ggml-cpu.lib
        "Vendor/whisper.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-vulkan/%{cfg.buildcfg}",     -- ggml-vulkan.lib

    }

    links {
        "whisper",
        "ggml",
        "ggml-base",
        "ggml-cpu",
        "ggml-vulkan",
        "%{Library.Vulkan}"
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        buildoptions { "/utf-8", "/Wall" }
        defines { 
            "_WINDOWS",
            "_USRDLL",
            "VB_EXPORT_STT" -- Export dll symbols
        }
        

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }
        links {
            "msvcprtd" -- To prevent LNK2019 errors from the regex & algorithm headers that whisper uses
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }
        links {
            "msvcprt" -- To prevent LNK2019 errors from the regex & algorithm headers that whisper uses
        }