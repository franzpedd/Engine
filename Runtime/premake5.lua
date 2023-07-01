project "Runtime"
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
        "%{includelist.GLM}"
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
        defines { "RUNTIME_DEBUG" }
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        defines { "RUNTIME_RELEASE" }
        runtime "Release"
        optimize "Full"

    filter "system:windows"
        disablewarnings { "4251", "26495", "26451", "33010" }