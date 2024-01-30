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
includelist["ImGUIzmo"] = "%{wks.location}/Thirdparty/imguizmo"
includelist["GLFW"] = "%{wks.location}/Thirdparty/glfw/include"
includelist["GLI"] = "%{wks.location}/Thirdparty/gli"
includelist["GLM"] = "%{wks.location}/Thirdparty/glm"
includelist["EnTT"] = "%{wks.location}/Thirdparty/entt/single_include"
includelist["JSON"] = "%{wks.location}/Thirdparty/json/single_include"
includelist["STB"] = "%{wks.location}/Thirdparty/stb"
includelist["Assimp"] = "%{wks.location}/Thirdparty/assimp/include"

-- libraries used
librarylist = {}
librarylist["Vulkan"] = "%{vkpath}/Lib/vulkan-1.lib"
librarylist["Shaderc"] = "%{vkpath}/Lib/shaderc_shared.lib"
librarylist["Assimp"] = "%{wks.location}/Thirdparty/assimp/lib"
librarylist["AssimpZLIB"] = "%{wks.location}/Thirdparty/assimp/contrib/zlib/"

-- include any dependency
group "Thirdarty"
    include "Thirdparty/entt.lua"
    include "Thirdparty/imgui.lua"
    include "Thirdparty/imguizmo.lua"
    include "Thirdparty/glfw.lua"
    include "Thirdparty/gli.lua"
    include "Thirdparty/glm.lua"
    include "Thirdparty/json.lua"
group ""

include "Engine"
include "Editor"