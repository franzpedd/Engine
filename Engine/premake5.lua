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
        "Source/**.cpp",

        "Thirdparty/**.c",
        "Thirdparty/**.h",
        "Thirdparty/**.hpp",
        "Thirdparty/**.cpp"
    }

    includedirs
    {
        "%{includelist.Engine}/Source",
        "%{includelist.Engine}/Thirdparty",

        "%{includelist.Vulkan}",
        "%{includelist.GLFW}",
        "%{includelist.GLM}",
        "%{includelist.Assimp}",
        "%{includelist.ImGui}"
    }

    links
    {
        "%{librarylist.Vulkan}",
        "%{librarylist.Shaderc}",
        "ImGui",
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
        optimize "On"

    filter "system:windows"
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS",
            "PLATFORM_WINDOWS"
        }

        linkoptions { "/ignore:4006" }

    filter {"configurations:Release", "system:windows"}
        links
        {
            "%{librarylist.Assimp}/Release/assimp-vc143-mt.lib",
            "%{librarylist.AssimpZLIB}/Release/zlibstatic.lib"
        }

    filter {"configurations:Debug", "system:windows"}
        links
        {
            "%{librarylist.Assimp}/Debug/assimp-vc143-mtd.lib",
            "%{librarylist.AssimpZLIB}/Debug/zlibstaticd.lib"
        }