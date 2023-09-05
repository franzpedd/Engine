project "Engine"
    characterset "ASCII"
    kind "StaticLib"
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

        "%{includelist.Vulkan}",
        "%{includelist.ImGUI}",
        "%{includelist.GLFW}",
        "%{includelist.GLI}",
        "%{includelist.GLM}",
        "%{includelist.TinyGLTF}",
		"%{includelist.STB}"
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
        defines 
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

        linkoptions { "/ignore:4006" }