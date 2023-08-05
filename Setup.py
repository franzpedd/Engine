
import os;
import sys;
import platform;
import subprocess;
import shutil;
import requests;
import tqdm;

class Util :

    @classmethod
    def Download(itself, url, path) :

        if(os.path.isfile(path) is True) : return;

        request = requests.get(url, stream = True);
        percentage = int(request.headers.get("content-lenght", 0));

        with open(path, "wb") as file, tqdm.tqdm(desc = path, total = percentage, unit = "iB", unit_scale = True, unit_divisor = 1024) as bar :
            for data in request.iter_content(chunk_size = 1024) :
                size = file.write(data);
                bar.update(size);

    @classmethod
    def Clone(itself, url, folder, version) :

        if(os.path.isdir("Thirdparty") is False) : os.makedirs("Thirdparty");
        if(os.path.isdir(f"Thirdparty/{folder}") is True) : return;

        scriptdir = os.getcwd();
        os.chdir("Thirdparty");
        subprocess.call(f"git clone {url} {folder}", shell = True);
        os.chdir(folder);
        subprocess.call(f"git checkout {version}", shell = True);
        os.chdir(scriptdir);
        
    @classmethod
    def Decompress(itself, file, path) :

        if(os.path.isfile(file) is None) : return;
        shutil.unpack_archive(file, path);

class Vulkan : 

    @classmethod
    def Download(itself, version) :

        if(os.getenv("VULKAN_SDK") is not None) : return;
        if(os.path.isdir("Thirdparty/vulkan-sdk") is False) : os.makedirs("Thirdparty/vulkan-sdk");

        url = "";
        path = "";

        if(platform.system() == "Windows") :
            url = f"https://sdk.lunarg.com/sdk/download/{version}/windows/VulkanSDK-{version}-Installer.exe";
            path = f"Thirdparty/vulkan-sdk/VulkanSDK-{version}-Installer.exe";
            
        if(platform.system() == "Linux") :
            url = f"https://sdk.lunarg.com/sdk/download/{version}/linux/vulkansdk-linux-x86_64-{version}.tar.gz";
            path = f"Thirdparty/vulkan-sdk/vulkansdk-linux-x86_64-{version}.tar.gz";

        if(platform.system() == "Darwin") :
            url = f"https://sdk.lunarg.com/sdk/download/{version}/mac/vulkansdk-macos-{version}.dmg";
            path = f"Thirdparty/vulkan-sdk/vulkansdk-macos-{version}.dmg";
        
        Util.Download(url, path);
        os.startfile(os.path.abspath(path));

class Premake :

    @classmethod
    def Download(itself, version) :

        if(os.path.isdir("Thirdparty/premake") is False) : os.makedirs(f"Thirdparty/premake");

        url = "";
        path = "";

        if(platform.system() == "Windows") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip";
            path = f"Thirdparty/premake/premake-{version}-windows.zip";

        if(platform.system() == "Linux") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-linux.tar.gz";
            path = f"Thirdparty/premake/premake-{version}-linux.tar.gz";

        if(platform.system() == "Darwin") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-macosx.tar.gz";
            path = f"Thirdparty/premake/premake-{version}-macosx.tar.gz";

        Util.Download(url, path);
        Util.Decompress(path, "Thirdparty/premake");
    
    @classmethod
    def Generate(itself) :

        if(len(sys.argv) == 2) :
            param_1= sys.argv[1];
            subprocess.call(f"Thirdparty/premake/premake5 {param_1}");

        else :

            if(platform.system() == "Windows") : subprocess.call("Thirdparty/premake/premake5 vs2022");
            if(platform.system() == "Linux") : subprocess.call("{Thirdparty/premake/premake5 gmake2");
            if(platform.system() == "Darwin") : subprocess.call("Thirdparty/premake/premake5 xcode4");

# entrypoint
Vulkan.Download("1.3.236.0");
Util.Clone("https://github.com/ocornut/imgui", "imgui", "docking");
Util.Clone("https://github.com/glfw/glfw", "glfw", "3.3-stable");
Util.Clone("https://github.com/g-truc/glm", "glm", "0.9.8");
Util.Clone("https://github.com/g-truc/gli", "gli", "0.8.2");
Util.Clone("https://github.com/syoyo/tinygltf", "tinygltf", "release");
Premake.Download("5.0.0-beta2");
Premake.Generate();