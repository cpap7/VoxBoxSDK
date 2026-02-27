workspace "VoxBoxSDK"
    configurations 
	{ 
		"Debug", 
		"Release", 
	--	"Dist"
	}
    platforms { "x64" }

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
	startproject "VoxBox-PiperApp"

    filter {}
	
include "External.lua"

--include "VoxBox-LlamaApp"
include "VoxBox-PiperApp"
--include "VoxBox-WhisperApp"

include "VoxBox-LlamaAPI"
include "VoxBox-PiperAPI"
include "VoxBox-WhisperAPI"

include "VoxBox-Common"