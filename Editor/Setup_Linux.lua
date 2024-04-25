-- linux premake5 script
project "Editor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir(dir)
    objdir(obj)

    files
    {
        "Source/**.cpp",
        "Source/**.h"
    }
    
    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Utility",

        "%{includes.Editor}/Source",
        "%{includes.GLM}",
        "%{includes.ImGui}",
        "%{includes.ImGuiExtra}",
        "%{includes.EnTT}",
        "%{includes.STB}"
    }

    links
    {
        "Engine",
        "ImGui"
    }

    
    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"

        links 
        {
            "vulkan",
            "shaderc_shared",
            "SDL2"
        }

        prebuildcommands
        {
            "{COPYDIR} %{wks.location}/Data " .. dir; 
        }

    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"
    
        links 
        {
            "vulkan",
            "shaderc_shared",
            "SDL2"
        }

        prebuildcommands
        {
            "{COPYDIR} %{wks.location}/Data " .. dir; 
        }