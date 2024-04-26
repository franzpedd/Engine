-- windows premake5 script
project "Editor"
    location "../"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"

    targetdir(dir)
    objdir(obj)
    vulkan_path = os.getenv("VULKAN_SDK");

    files
    {
        "Source/**.cpp",
        "Source/**.h"
    }
    
    includedirs
    {
        "%{includes.Engine}/Source",
        "%{includes.Engine}/Wrapper ",

        "%{includes.Editor}/Source",
        "%{includes.GLM}",
        "%{includes.ImGui}",
        "%{includes.ImGuiExtra}",
        "%{includes.EnTT}",
        "%{includes.STB}",
        "%{includes.Assimp}",

        "%{vulkan_path}/Include",
        "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/include",
        "%{wks.location}/Thirdparty/assimp/include",
        "%{wks.location}/Thirdparty/openal/include"
    }

    links
    {
        "Engine",
        "ImGui"
    }
    
    filter "configurations:Debug"
        defines { "EDITOR_DEBUG" }
        runtime "Debug"
        symbols "On"

        links
        {
            -- vulkan
            "%{vulkan_path}/Lib/vulkan-1.lib",
            "%{vulkan_path}/Lib/shaderc_shared.lib",
            -- sdl
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.lib",
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2main.lib",
            -- assimp
            "%{wks.location}/Thirdparty/assimp/build/Debug/lib/Debug/assimp-vc143-mtd.lib",
            "%{wks.location}/Thirdparty/assimp/build/Debug/contrib/zlib/Debug/zlibstaticd.lib",
            -- open-al
            "%{wks.location}/Thirdparty/openal/build/Debug/Debug/OpenAL32.lib"
        }

        defines
        {
            "_CRT_SECURE_NO_WARNINGS"
        }

    filter "configurations:Release"
        defines { "EDITOR_RELEASE" }
        runtime "Release"
        optimize "On"

        links
        {
            -- vulkan
            "%{vulkan_path}/Lib/vulkan-1.lib",
            "%{vulkan_path}/Lib/shaderc_shared.lib",
            -- sdl
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2.lib",
            "%{wks.location}/Thirdparty/sdl/SDL2-2.30.2/lib/x64/SDL2main.lib",
            -- assimp
            "%{wks.location}/Thirdparty/assimp/build/Release/lib/Release/assimp-vc143-mtd.lib",
            "%{wks.location}/Thirdparty/assimp/build/Release/contrib/zlib/Release/zlibstaticd.lib",
            -- open-al
            "%{wks.location}/Thirdparty/openal/build/Release/Release/OpenAL32.lib"
        }