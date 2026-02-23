-- premake5.lua
project "VoxBox-PiperAPI"
    kind "SharedLib"
    language "C++"
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")
    
    pchheader "vbppch.h"
    pchsource "Source/vbppch.cpp"
    filter "files:Vendor/piper/src/cpp/**.cpp"
        flags { "NoPCH" }
    filter{}

    files {
        "Source/**.cpp",
        "Source/**.h",
        "Vendor/piper/src/cpp/piper.cpp"
    }

    includedirs {
        "Source",
        "../VoxBox-Shared",
        "Vendor/piper/build/pi/include",
        "Vendor/piper/src/cpp",
        "Vendor/piper/build/si/include",
    }

    filter "system:windows"
        systemversion "latest"
        cppdialect "C++20"
        defines { "_WINDOWS", "_USRDLL" , "VB_EXPORT_TTS" }
        flags { "MultiProcessorCompile" }
        vectorextensions "AVX2"
        openmp "On"
        buildoptions { "/utf-8", "/Wall" }   -- /utf-8 since piper uses spdlog
        disablewarnings { "4711", "5045" }   -- Disable a couple warnings 
        libdirs { "Vendor/piper/build/pi/lib" }
        links {
            "espeak-ng",
            "piper_phonemize",
            "onnxruntime",
        }
        linkoptions { "/WX" }               -- Treat linker warnings as errors


    filter "system:linux"
        cppdialect "C++17" -- per CMakeLists.txt
        pic "On"
        libdirs { "Vendor/piper/build/pi/lib" }
        links {
            "espeak-ng",
            "onnxruntime",
            "piper_phonemize",
        }

    filter "configurations:Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }
        symbols "Off"