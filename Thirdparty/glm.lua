project "GLM"
    location "glm"
    kind "None"
    language "C++"
    cppdialect "C++11"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "glm/**.hpp",
        "glm/**.h"
    }

    includedirs
    {
        "glm"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"