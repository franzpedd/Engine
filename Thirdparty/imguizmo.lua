project "ImGUIzmo"
    location "imguizmo"
    kind "None"
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
        "imguizmo"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"