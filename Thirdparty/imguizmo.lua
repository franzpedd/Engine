project "ImGUIzmo"
    location "imguizmo"
    kind "StaticLib"
    language "C++"
    cppdialect "C++11"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "imguizmo/imguizmo.h",
        "imguizmo/imguizmo.cpp"
    }

    includedirs
    {
        "imguizmo",
        "%{includelist.ImGUI}"
    }

    links
    {
        "ImGUI"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    filter "system:windows"
        disablewarnings { "26495", "6255", "6001", "6263" }