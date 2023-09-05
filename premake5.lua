workspace "Solution"
architecture "x86_64"
configurations { "Debug", "Release" }
startproject "Editor"

-- global build path and global vulkan directory
dirpath = "%{wks.location}/_Build/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
objpath = "%{wks.location}/_Build/temp/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
vkpath = os.getenv("VULKAN_SDK")

-- includes directory
includelist = {}
includelist["Engine"] = "%{wks.location}/Engine/Source"
includelist["Editor"] = "%{wks.location}/Editor/Source"
includelist["Vulkan"] = "%{vkpath}/Include"
includelist["ImGUI"] = "%{wks.location}/Thirdparty/imgui"
includelist["GLFW"] = "%{wks.location}/Thirdparty/glfw/include"
includelist["GLI"] = "%{wks.location}/Thirdparty/gli"
includelist["GLM"] = "%{wks.location}/Thirdparty/glm"
includelist["TinyGLTF"] = "%{wks.location}/Thirdparty/tinygltf"
includelist["STB"] = "%{wks.location}/Thirdparty/stb"

-- libraries used
librarylist = {}
librarylist["Vulkan"] = "%{vkpath}/Lib/vulkan-1.lib"
librarylist["Shaderc"] = "%{vkpath}/Lib/shaderc_shared.lib"

-- include any dependency
group "Thirdarty"
    include "Thirdparty/imgui.lua"
    include "Thirdparty/glfw.lua"
    include "Thirdparty/gli.lua"
    include "Thirdparty/glm.lua"
    include "Thirdparty/tinygltf.lua"
group ""

include "Engine"
include "Editor"