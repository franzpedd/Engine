project "Game"
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
        "%{includelist.Engine}/Source",
        "%{includelist.Engine}/Thirdparty",
        "%{includelist.Game}/Source",
        
        "%{includelist.Vulkan}",
        "%{includelist.GLM}",
        "%{includelist.Assimp}",
        "%{includelist.ImGui}"
    }

    links
    {
        "Engine",
        "ImGui"
    }

    defines 
    {
        "API_IMPORT"
    }

    filter "configurations:Debug"
        defines { "GAME_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "GAME_RELEASE" }
        runtime "Release"
        optimize "On"

    filter "system:windows"
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS",
            "PLATFORM_WINDOWS"
        }

        linkoptions { "/ignore:4006" }