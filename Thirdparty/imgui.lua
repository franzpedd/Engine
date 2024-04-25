project "ImGui"
    location "imgui"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "imgui/imconfig.h",
        "imgui/imgui_demo.cpp",
        "imgui/imgui_draw.cpp",
        "imgui/imgui_internal.h",
        "imgui/imgui_tables.cpp",
        "imgui/imgui_widgets.cpp",
        "imgui/imgui.h",
        "imgui/imgui.cpp",
        "imgui/imstb_rectpack.h",
        "imgui/imstb_textedit.h",
        "imgui/imstb_truetype.h"
    }

    includedirs
    {
        "imgui"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"
    
    filter "configurations:Release"
        warnings "Default"
        runtime "Release"
        optimize "On"