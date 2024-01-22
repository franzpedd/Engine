project "TinyGLTF"
    location "tinygltf"
    kind "None"
    language "C++"
    cppdialect "C++11"

    targetdir(dirpath)
    objdir(objpath)

    files
    {
        "tinygltf/json.hpp",
        "tinygltf/stb_image.h",
        "tinygltf/stb_image_write.h",
        "tinygltf/tiny_gltf.h"
    }

    includedirs
    {
        "tinygltf"
    }

    filter "configurations:Debug"
        warnings "Off"
        runtime "Debug"
        symbols "On"

    filter "configurations:Release"
        runtime "Release"
        optimize "Full"

    filter "system:windows"
        disablewarnings { "26495" }