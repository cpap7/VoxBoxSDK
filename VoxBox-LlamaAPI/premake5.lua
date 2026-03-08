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
        "Vendor/llama.cpp/include",                  -- llama headers
        "Vendor/llama.cpp/ggml/include",             -- ggml headers
        "Vendor/llama.cpp/common",                   -- common.h
        "Vendor/llama.cpp/src",                      
        "Vendor/llama.cpp/vendor",                   -- bundled third-party deps
    }

    -- Prebuilt llama.cpp libs 
    libdirs {
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/src/%{cfg.buildcfg}",                     -- llama.lib
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/ggml/src/%{cfg.buildcfg}",                -- ggml.lib
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-base/%{cfg.buildcfg}",      -- ggml-base.lib
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-cpu/%{cfg.buildcfg}",       -- ggml-cpu.lib
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/ggml/src/ggml-vulkan/%{cfg.buildcfg}",    -- ggml-vulkan.lib
        "Vendor/llama.cpp/%{cfg.buildcfg:lower()}/common/%{cfg.buildcfg}",                  -- common.lib
    }

    links {
        "llama",
        "ggml",
        "ggml-base",
        "ggml-cpu",
        "ggml-vulkan",
        "common",
        "%{Library.Vulkan}",
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