project "Engine"
    characterset "ASCII"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    pchheader "epch.h"
    pchsource "Source/epch.cpp"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "Source/**.c",
        "Source/**.h",
        "Source/**.hpp",
        "Source/**.cpp"
    }

    includedirs
    {
        "%{includelist.Engine}",

        "%{includelist.Vulkan}",
        "%{includelist.ImGUI}",
        "%{includelist.ImGUIzmo}",
        "%{includelist.GLFW}",
        "%{includelist.GLI}",
        "%{includelist.GLM}",
        "%{includelist.TinyGLTF}",
        "%{includelist.EnTT}",
        "%{includelist.JSON}"
    }

    links
    {
        "%{librarylist.Vulkan}",
        "%{librarylist.Shaderc}",
        "ImGUI",
        "GLFW"
    }

    defines 
    {
        "API_EXPORT"
    }

    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "Full"

    filter "system:windows"
        optimize "Speed"
        
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS",
            "PLATFORM_WINDOWS"
        }

        linkoptions { "/ignore:4006" }