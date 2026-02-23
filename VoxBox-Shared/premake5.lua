-- premake5.lua
project "VoxBox-Shared"
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

    filter "system:windows"
        systemversion "latest"
        --characterset "Unicode"
        defines { 
            "_WINDOWS",
        }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"
        defines { "_DEBUG" }

    filter "configurations:Release"
        runtime "Release"
        optimize "On"
        defines { "NDEBUG" }