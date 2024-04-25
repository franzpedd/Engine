-- global specification
workspace "Solution"
architecture "x86_64"
configurations { "Debug", "Release" };
startproject "Editor"

---- paths for solution build
dir = "%{wks.location}/_build/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
obj = "%{wks.location}/_build/temp/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

---- projects and dependencies header directories
includes = {}
includes["Engine"] = "%{wks.location}/Engine"
includes["Editor"] = "%{wks.location}/Editor"
includes["GLM"] = "%{wks.location}/Thirdparty/glm"
includes["ImGui"] = "%{wks.location}/Thirdparty/imgui"
includes["ImGuiExtra"] = "%{wks.location}/Thirdparty/imgui_extra"
includes["EnTT"] = "%{wks.location}/Thirdparty/entt"
includes["STB"] = "%{wks.location}/Thirdparty/stb"

---- dependencies
group "Thirdpaty"
    include "Thirdparty/glm.lua"
    include "Thirdparty/imgui.lua"
group ""

---- projects by current os
if os.host() == "windows" then
    include "Editor/Setup_Windows.lua"
    include "Engine/Setup_Windows.lua"
end

if os.host() == "linux" then
    include "Editor/Setup_Linux.lua"
    include "Engine/Setup_Linux.lua"
end

if os.host() == "macosx" then -- not implemented
    include "Editor/Setup_MacOS.lua"
    include "Engine/Setup_MacOS.lua"
end