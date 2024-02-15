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
includelist["Engine"] = "%{wks.location}/Engine"
includelist["Editor"] = "%{wks.location}/Editor"
includelist["Vulkan"] = "%{vkpath}/Include"
includelist["GLFW"] = "%{wks.location}/Thirdparty/glfw/include"
includelist["GLM"] = "%{wks.location}/Thirdparty/glm"
includelist["Assimp"] = "%{wks.location}/Thirdparty/assimp/include"
includelist["ImGui"] = "%{wks.location}/Thirdparty/imgui"

-- libraries used
librarylist = {}
librarylist["Vulkan"] = "%{vkpath}/Lib/vulkan-1.lib"
librarylist["Shaderc"] = "%{vkpath}/Lib/shaderc_shared.lib"

librarylist["Assimp"] = "%{wks.location}/Thirdparty/assimp/lib/"
librarylist["AssimpZLIB"] = "%{wks.location}/Thirdparty/assimp/contrib/zlib/"

-- include any dependency project
group "Thirdarty"
    include "Thirdparty/glfw.lua"
    include "Thirdparty/glm.lua"
    include "Thirdparty/imgui.lua"
group ""

include "Engine"
include "Editor"