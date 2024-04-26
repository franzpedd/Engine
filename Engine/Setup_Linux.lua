-- linux premake5 script
project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"

    pchheader "epch.h"
    pchsource "Source/epch.cpp"

    targetdir(dir)
    objdir(dir)

    files
    {
        "Source/**.h",
        "Source/**.cpp",

        "Wrapper/**.h"
    }

    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Wrapper",
        "%{includes.GLM}",
        "%{includes.ImGui}",
        "%{includes.ImGuiExtra}",
        "%{includes.EnTT}",
        "%{includes.STB}",

        "%{wks.location}/Thirdparty/assimp/include",
        "%{wks.location}/Thirdparty/openal/include"
    }

    links
    {
        "ImGui"
    }

    filter "configurations:Debug"
        defines { "ENGINE_DEBUG" }
        runtime "Debug"
        symbols "On"

        prebuildcommands
        {
            "{COPYFILE} %{wks.location}/Thirdparty/openal/build/Debug/libopenal.so " .. dir;
        }

        libdirs
        {
            "%{wks.location}/Thirdparty/assimp/build/Debug/lib",
            "%{wks.location}/Thirdparty/assimp/build/Debug/contrib/zlib",
            "%{wks.location}/Thirdparty/openal/build/Debug"
        }

        links
        {
            "vulkan",
            "shaderc_shared",
            "SDL2",

            -- assimp
            "assimp",
            "zlibstatic",
            --openal
            "openal"
        }

    filter "configurations:Release"
        defines { "ENGINE_RELEASE" }
        runtime "Release"
        optimize "On"

        prebuildcommands
        {
            "{COPYFILE} %{wks.location}/Thirdparty/openal/build/Release/libopenal.so " .. dir;
        }

        libdirs
        {
            "%{wks.location}/Thirdparty/assimp/build/Release/lib",
            "%{wks.location}/Thirdparty/assimp/build/Release/contrib/zlib",
            "%{wks.location}/Thirdparty/openal/build/Release"
        }

        links 
        {
            "vulkan",
            "shaderc_shared",
            "SDL2",
            
            -- assimp
            "assimp",
            "zlibstatic",
            --openal
            "openal"
        }