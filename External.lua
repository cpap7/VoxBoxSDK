-- For this, we'll assume you have Vulkan 1.3.283.0 installed (llama.cpp targets this currently)
VULKAN_SDK = os.getenv("VULKAN_SDK")

-- Additionally, we'll assume OpenSSL was installed via vcpkg
-- Ensure VCPKG_ROOT is defined as a system environment variable
VCPKG_ROOT = os.getenv("VCPKG_ROOT") or "C:/vcpkg"

-- Dependency folders relative to the root folder
IncludeDir = {}
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
--IncludeDir["LlamaLib"] = "../Vendor/LlamaLib/include"
IncludeDir["VCPKG"] = "%{VCPKG_ROOT}/installed/x64-windows/include" -- Zlib, Curl, OpenSSL

-- Binary libs relative to the root folder
LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VCPKG"] = "%{VCPKG_ROOT}/installed/x64-windows/lib" -- Zlib, Curl, OpenSSL

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"


group "Dependencies"
   --include "VoxBox-Llama/Vendor/llama"
   --include "VoxBox-Piper/Vendor/piper"
   --include "VoxBox-Whisper/Vendor/whisper"
group ""

group "Applications"
--   include "VoxBox-LlamaApp"
   include "VoxBox-PiperApp"
--   include "VoxBox-WhisperApp"
group ""

group "APIs"
	include "VoxBox-LlamaAPI"
	include "VoxBox-PiperAPI"
	include "VoxBox-WhisperAPI"
group ""

group "Misc"
	include "VoxBox-Shared"
group ""