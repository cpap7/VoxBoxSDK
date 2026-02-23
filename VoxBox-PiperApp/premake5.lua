-- premake5.lua
project "VoxBox-PiperApp"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    
    targetdir ("bin/%{cfg.buildcfg}-%{cfg.architecture}")
    objdir ("bin-int/%{cfg.buildcfg}-%{cfg.architecture}")

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "../VoxBox-PiperAPI/Source/API",        -- For VoxBoxTTS.h
    }

    libdirs {
        "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}",  -- VoxBox-Piper.lib
    }

    links {
        "VoxBox-PiperAPI",
    }

    postbuildcommands {
        '{COPYDIR} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/espeak" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/espeak-ng.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/onnxruntime.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/VoxBox-PiperAPI.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/piper_phonemize.dll" "%{cfg.targetdir}"',
        '{COPYFILE} "../VoxBox-PiperAPI/bin/%{cfg.buildcfg}-%{cfg.architecture}/piper.exe" "%{cfg.targetdir}"',
    }

    filter "system:windows"
        systemversion "latest"
        defines { "_WINDOWS" }

    filter "configurations:Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        optimize "On"
        defines { "NDEBUG" }