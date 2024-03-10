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
        "%{includelist.Engine}/Source",
        "%{includelist.Engine}/Thirdparty",
        "%{includelist.Editor}/Source",
        
        "%{includelist.Vulkan}",
        "%{includelist.GLM}",
        "%{includelist.Assimp}",
        "%{includelist.ImGui}",
        "%{includelist.OpenAL}"
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
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"

    filter "system:windows"
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS",
            "PLATFORM_WINDOWS"
        }

        linkoptions { "/ignore:4006" }