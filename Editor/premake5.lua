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
        "%{includelist.GLI}",
        "%{includelist.GLM}",
        "%{includelist.TinyGLTF}"
    }

    links
    {
        "Engine"
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
            "_CRT_SECURE_NO_WARNINGS"
        }

        linkoptions { "/ignore:4006", "/ignore:4098"  }