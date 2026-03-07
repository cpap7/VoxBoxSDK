-- premake5.lua
project "VoxBox-LlamaAPI"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"
    staticruntime "Off" -- /MD
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    -- Precompiled header
    pchheader "vblpch.h"
    pchsource "Source/vblpch.cpp"

    files {
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.c",
        "Source/**.cpp",
    }

    includedirs {
        "Source",                                   
        "../VoxBox-Common/Source",                   -- VBCommon.h
        "Vendor/llama.cpp/include",                  -- llama.h
        "Vendor/llama.cpp/ggml/include",             -- ggml headers
        "Vendor/llama.cpp/common",                   -- common.h
        "Vendor/llama.cpp/src",                      -- internal llama headers
    }

    -- Prebuilt llama.cpp libs (Ninja single-config: debug/<target>/lib)
    libdirs {
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/src",
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/ggml/src",
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/common",
    }

    links {
        "llama",
        "ggml",
        "ggml-base",
        "ggml-cpu",
        "common",
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        buildoptions { "/utf-8", "/Wall" }
        defines {
            "_CRT_SECURE_NO_WARNINGS",
            "_WINDOWS",
            "_USRDLL",
            "VB_EXPORT_LLM",
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }
        links {
            "msvcprtd" -- Prevent LNK2019 from STL headers used by llama common
        }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }
        links {
            "msvcprt"
        }

    filter "system:linux"
        pic "On"
        libdirs {
            "Vendor/llama.cpp/release/src",
            "Vendor/llama.cpp/release/ggml/src",
            "Vendor/llama.cpp/release/common",
        }
        links {
            "llama",
            "ggml",
            "ggml-base",
            "ggml-cpu",
            "common",
        }
    filter {}