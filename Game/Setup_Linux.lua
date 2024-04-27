-- linux premake5 script
project "Game"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir(dir)
    objdir(obj)

    files
    {
        "Source/**.cpp",
        "Source/**.h"
    }
    
    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Wrapper",

        "%{includes.Game}/Source",
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
        "Engine",
        "ImGui"
    }
    
    filter "configurations:Debug"
        defines { "GAME_DEBUG" }
        runtime "Debug"
        symbols "On"

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

        prebuildcommands
        {
            "{COPYDIR} %{wks.location}/Data " .. dir; 
        }

    filter "configurations:Release"
        defines { "GAME_RELEASE" }
        runtime "Release"
        optimize "On"

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

        prebuildcommands
        {
            "{COPYDIR} %{wks.location}/Data " .. dir; 
        }