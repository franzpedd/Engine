project "entt"
    location "entt"
    kind "None"
    language "C++"
    cppdialect "C++11"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "entt/single_include/**.hpp"
    }

    includedirs
    {
        "entt"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"