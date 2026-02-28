-- premake5.lua
project "VoxBox-Common"
    kind "Utility"
    language "C++"
    cppdialect "C++20"

    files {
        "Source/**.h",
        "Source/**.cpp"
    }

    includedirs {
        "Source", 
    }

    defines {
      "VB_STT_EXPORT",
      "VB_TTS_EXPORT",
      "VB_LLM_EXPORT"
    }

    filter "system:windows"
        systemversion "latest"
        toolset "v145" -- VS 2026
        defines { 
            "_WINDOWS"
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }