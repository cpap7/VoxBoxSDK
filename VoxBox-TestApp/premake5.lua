-- premake5.lua
project "VoxBox-TestApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    files {
        "Source/**.h",
        "Source/**.c",
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs {
        "../VoxBox-PiperAPI/Source/API",        -- For VoxBoxTTS.h
        "../VoxBox-WhisperAPI/Source/API",      -- For VoxBoxSTT.h
        "../VoxBox-Common/Source",              -- For VBCommon.h
    }

    libdirs {
        "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}",  -- VoxBox-Piper.lib
        "../VoxBox-WhisperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}",  -- VoxBox-WhisperAPI.lib

    }

    links {
        "VoxBox-PiperAPI",
        "VoxBox-WhisperAPI",
    }

    postbuildcommands {
        -- Piper
        '{COPYDIR} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/espeak" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/espeak-ng.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/onnxruntime.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/VoxBox-PiperAPI.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/piper_phonemize.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/piper.exe" "%{cfg.targetdir}"',
        -- Whisper        
        '{COPYFILE} "../VoxBox-WhisperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/VoxBox-WhisperAPI.dll" "%{cfg.targetdir}"',

    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        defines { "_WINDOWS" }

    filter "configurations:Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }