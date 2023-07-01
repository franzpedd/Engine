workspace "Solution"
architecture "x86_64"
configurations { "Debug", "Release" }
startproject "Runtime"

-- global build path and global vulkan directory
dirpath = "%{wks.location}/Build/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
objpath = "%{wks.location}/Build/temp/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
vkpath = os.getenv("VULKAN_SDK")

-- includes directory
includelist = {}
includelist["Engine"] = "%{wks.location}/Engine/Source"
includelist["Runtime"] = "%{wks.location}/Runtime/Source"
includelist["GLI"] = "%{wks.location}/Thirdparty/gli"
includelist["GLM"] = "%{wks.location}/Thirdparty/glm"
includelist["Vulkan"] = "%{vkpath}/Include"

-- libraries used
librarylist = {}
librarylist["Vulkan"] = "%{vkpath}/Lib/vulkan-1.lib"
librarylist["Shaderc"] = "%{vkpath}/Lib/shaderc_shared.lib"

-- include any dependency
group "Thirdarty"
    include "Thirdparty/gli.lua"
    include "Thirdparty/glm.lua"
group ""

-- include projects
include "Engine"
include "Runtime"