project "GLI"
    location "gli"
    kind "None"
    language "C++"
    cppdialect "C++11"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "gli/**.hpp",
        "gli/**.h"
    }

    includedirs
    {
        "gli"
    }

    filter "configurations:Debug"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"