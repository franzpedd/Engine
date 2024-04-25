-- linux premake5 script
project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    pchheader "epch.h"
    pchsource "Source/epch.cpp"

    targetdir(dir)
    objdir(dir)

    files
    {
        "Source/**.h",
        "Source/**.cpp",

        "Utility/**.h"
    }

    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Utility",
        "%{includes.GLM}",
        "%{includes.ImGui}",
        "%{includes.ImGuiExtra}",
        "%{includes.EnTT}",
        "%{includes.STB}"
    }

    links
    {
        "ImGui"
    }

    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

        links
        {
            "vulkan",
            "shaderc_shared",
            "SDL2"
        }

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "On"

        links 
        {
            "vulkan",
            "shaderc_shared",
            "SDL2"
        }