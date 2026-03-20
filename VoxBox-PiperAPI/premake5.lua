-- premake5.lua
project "VoxBox-PiperAPI"
    kind "SharedLib"
    language "C++"
    cppdialect "C++20"

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
        "../VoxBox-Common/Source",
        "Vendor/piper/src/cpp",
    }

    links {
        "espeak-ng",
        "piper_phonemize",
        "onnxruntime",
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        defines { "_WINDOWS", "_USRDLL" , "VB_EXPORT_TTS" }
        flags { "MultiProcessorCompile" }
        vectorextensions "AVX2"
        openmp "On"
        buildoptions { "/utf-8", "/Wall" }   -- /utf-8 since piper uses spdlog
        disablewarnings { "4711", "5045" }   -- Disable a couple warnings 
        linkoptions { "/WX" }               -- Treat linker warnings as errors


    filter "system:linux"
        pic "On"
        

    filter "configurations:Debug"
        symbols "On"
        defines { "_DEBUG" }
        links {
            "msvcprtd"    
        }
        includedirs {
            "Vendor/piper/debug/pi/include",
            "Vendor/piper/debug/si/include",
        }
        libdirs { "Vendor/piper/debug/pi/lib" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }
        symbols "Off"
        links {
            "msvcprt"
        }
        includedirs {
            "Vendor/piper/release/pi/include",
            "Vendor/piper/release/si/include",    
        }
        libdirs { "Vendor/piper/release/pi/lib" }
