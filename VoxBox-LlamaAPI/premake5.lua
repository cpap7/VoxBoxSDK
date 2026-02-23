-- premake5.lua
project "VoxBox-LlamaAPI"
    kind "SharedLib"
    language "C++"

    -- Sources 
    files {
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.c",
        "Source/**.cpp",
    }

    -- Don't compile llama.cpp itself as part of this library (it's linked prebuilt)
    removefiles {
        "VoxBox-Llama/llama/**",
    }

    -- Common include dirs (matches vcxproj + Makefile)
    includedirs {
        "VoxBox-Llama/llama/common",
        "VoxBox-Llama/llama/ggml/include",
        "VoxBox-Llama/llama/include",
        "VoxBox-Llama/llama/src",
        "VoxBox-Llama/llama/vendor",
    }

    -- Common defines
    filter "system:windows"
        defines {
            "_CRT_SECURE_NO_WARNINGS",
            "VB_EXPORT_LLM",
            "VB_LLM_EXPORTS",
            "_WINDOWS",
            "_USRDLL",
        }
    filter {}

    -- Dialect: vcxproj uses C++20, Makefile uses C++17
    filter "system:windows"
        cppdialect "C++20"
    filter "not system:windows"
        cppdialect "C++17"
        pic "On"
    filter {}

    -- Config-specific defines
    filter "configurations:Debug"
        defines { "_DEBUG" }
    filter "configurations:Release"
        defines { "NDEBUG" }
    filter {}

    -------------------------------------------------------------------------
    -- Windows (x64): link against prebuilt llama.cpp libs
    -------------------------------------------------------------------------
    filter { "system:windows", "platforms:x64", "configurations:Debug" }
        libdirs {
            "Vendor/llama/build-x64-windows-msvc-debug/src",
            "Vendor/llama/build-x64-windows-msvc-debug/ggml/src",
            "Vendor/llama/build-x64-windows-msvc-debug/common",
        }
        links {
            "llama",
            "ggml",
            "ggml-base",
            "ggml-cpu",
            "common",
        }

    filter { "system:windows", "platforms:x64", "configurations:Release" }
        libdirs {
            "Vendor/llama/build-x64-windows-msvc-release/src",
            "Vendor/llama/build-x64-windows-msvc-release/ggml/src",
            "Vendor/llama/build-x64-windows-msvc-release/common",
        }
        links {
            "llama",
            "ggml",
            "ggml-base",
            "ggml-cpu",
            "common",
        }
    filter {}

    -------------------------------------------------------------------------
    -- Linux (and other gmake targets)
    -------------------------------------------------------------------------
    filter "system:linux"
        targetname "VoxBox-Llama" -- Makefile builds libmtllm.so
        libdirs {
            "Vendor/llama/build/bin",
            "Vendor/llama/build/common",
        }
        links {
            "ggml-base",
            "ggml-cpu",
            "ggml",
            "llama",
            "common",
        }
        -- If you build llama.cpp with CUDA and want it:
        -- links { "ggml-cuda" }
    filter {}