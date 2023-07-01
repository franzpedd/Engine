project "Engine"
    kind "SharedLib"
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
        "%{includelist.GLI}",
        "%{includelist.GLM}"
    }

    links
    {
        "%{librarylist.Vulkan}",
        "%{librarylist.Shaderc}"
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
        disablewarnings { "4251", "26495", "26451", "33010" }