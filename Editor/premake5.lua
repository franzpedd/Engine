project "Editor"
    characterset "ASCII"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

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
        "%{includelist.Editor}",
        
        "%{includelist.Vulkan}",
        "%{includelist.ImGUI}",
        "%{includelist.ImGUIzmo}",
        "%{includelist.GLI}",
        "%{includelist.GLM}",
        "%{includelist.EnTT}",
        "%{includelist.JSON}",
        "%{includelist.STB}",
        "%{includelist.Assimp}"
    }

    links
    {
        "Engine",
        "ImGUI",
        "ImGUIzmo"
    }

    defines 
    {
        "API_IMPORT"
    }

    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "Full"

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
            "%{librarylist.Assimp}/Release/assimp-vc143-mtd.lib",
            "%{librarylist.AssimpZLIB}/Release/zlibstaticd.lib"
        }

    filter {"configurations:Debug", "system:windows"}
        links
        {
            "%{librarylist.Assimp}/Debug/assimp-vc143-mtd.lib",
            "%{librarylist.AssimpZLIB}/Debug/zlibstaticd.lib"
        }