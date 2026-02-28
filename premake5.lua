workspace "VoxBoxSDK"
    configurations 
	{ 
		"Debug", 
		"Release", 
	--	"Dist"
	}
    platforms { "x64" }

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	startproject "VoxBox-TestApp"

    filter {}
	
include "External.lua"

include "VoxBox-TestApp"

include "VoxBox-LlamaAPI"
include "VoxBox-PiperAPI"
include "VoxBox-WhisperAPI"

include "VoxBox-Common"