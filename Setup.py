# default libraries
import os;
import sys;
import platform;
import subprocess;
import shutil;

# custom;/dependencies
import requests;
import tqdm;

# utilities class
class Util :

    @classmethod
    def Download(itslf, url, path) :

        if(os.path.isfile(path) is True) :
            print(f"{path} already downloaded, skipping...");
            return;
        
        request = requests.get(url, stream=True);
        percentage = int(request.headers.get("content-lenght", 0));
        
        with open(path, "wb") as file, tqdm.tqdm(desc = path, total = percentage, unit = "iB", unit_scale = True, unit_divisor = 1024) as bar :
            for data in request.iter_content(chunk_size = 1024) :
                size = file.write(data);
                bar.update(size);
    
    @classmethod
    def Clone(itself, url, folder, version) :
        
        if(os.path.isdir("Thirdparty") is False) :
            print("Creating Thirdparty folder");
            os.makedirs("Thirdparty");

        if(os.path.isdir(f"Thirdparty/{folder}") is True) :
            print(f"{folder} already exists, skipping...");
            return;
        
        scriptDir = os.getcwd();
        os.chdir("Thirdparty");
        subprocess.call(f"git clone {url} {folder}", shell = True);
        os.chdir(folder);
        subprocess.call(f"git checkout {version}", shell = True);
        os.chdir(scriptDir);

    @classmethod
    def Decompress(itself, file, path) :

        if(os.path.isfile(file) is None) :
            print(f"Decompress Error: {file} doesnt exists, skipping...");
            return;
        
        shutil.unpack_archive(file, path);

# premake is a solution generateor for linux, windows, mac and others
class Premake :

    @classmethod
    def Download(itself, version) :

        if(os.path.isdir("Thirdparty/premake") is False) :
            os.makedirs("Thirdparty/premake");
        
        url = "";
        path = "";

        if(platform.system() == "Linux") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-linux.tar.gz";
            path = f"Thirdparty/premake/premake-{version}-linux.tar.gz";

        if(platform.system() == "Windows") :
            url = f"https://github.com/premake/premake-core/releases/download/v{version}/premake-{version}-windows.zip";
            path = f"Thirdparty/premake/premake-{version}-windows.zip";

        Util.Download(url, path);
        Util.Decompress(path, "Thirdparty/premake");
    
    @classmethod
    def GenerateSolution(itself) :

        if(len(sys.argv) == 2) :
            subprocess.call(f"Thirdparty/premake/premake5 {sys.argv[1]}", shell = True);
            return;

        if(platform.system() == "Linux") : 
            subprocess.call(f"Thirdparty/premake/premake5 gmake2", shell = True);

        if(platform.system() == "Windows") :
            subprocess.call(f"Thirdparty/premake/premake5 vs2022");


class Dependencies:

    # vulkan renderer api, under linux distros it must be downloaded as a package
    @classmethod
    def DownloadVulkan(itself, version) :

        if(platform.system() == "Windows") :

            if(os.getenv("VULKAN_SDK") is not None) : 
                return;
        
            if(os.path.isdir("Thirdparty/vulkan-sdk") is False) : 
                os.makedirs("Thirdparty/vulkan-sdk");

            url = f"https://sdk.lunarg.com/sdk/download/{version}/windows/VulkanSDK-{version}-Installer.exe";
            path = f"Thirdparty/vulkan-sdk/VulkanSDK-{version}-Installer.exe";
            Util.Download(url, path);
            os.startfile(os.path.abspath(path));

    # sdl windowing manager, under linux distros it must be downloaded as a package
    @classmethod
    def DownloadSDL(itself, version) :
            
        if(platform.system() == "Windows") :
        
            if(os.path.isdir("Thirdparty/sdl") is False) : 
                os.makedirs("Thirdparty/sdl");

            url = f"https://sdk.lunarg.com/sdk/download/{version}/windows/VulkanSDK-{version}-Installer.exe";
            url = f"https://github.com/libsdl-org/SDL/releases/download/release-{version}/SDL2-devel-{version}-VC.zip";
            path = f"Thirdparty/sdl/SDL2-devel-{version}-VC.zip";

            Util.Download(url, path);
            Util.Decompress(path, "Thirdparty/sdl");

    # assimp model importer, used to import gltf models and maybe other formats in the future
    @classmethod
    def DownloadAssimp(itself, version) : 

        Util.Clone("https://github.com/assimp/assimp", "assimp", version);

        if((os.path.isdir("Thirdparty/assimp/lib") is True)) :
            return;

        print("\n------------------------Building Assimp------------------------\n");

        scriptdir = os.getcwd();
        os.chdir("Thirdparty/assimp");

        params = '''-DBUILD_SHARED_LIBS=OFF 
                    -DASSIMP_BUILD_ZLIB=ON 
                    -DASSIMP_NO_EXPORT=ON 
                    -DASSIMP_BUILD_TESTS=OFF 
                    -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF 
                    -DASSIMP_BUILD_GLTF_IMPORTER=ON '''

        if(platform.system() == "Windows") :
            subprocess.call(f"cmake CMakeLists.txt {params}", shell = True);
            subprocess.call(f"cmake --build . --config Debug", shell = True);
            subprocess.call(f"cmake --build . --config Release", shell = True);
        
        os.chdir(scriptdir);
        print("\n------------------------Assimp building has finished------------------------\n");

# main
Dependencies.DownloadVulkan("1.3.236.0");
Dependencies.DownloadSDL("2.30.2");
Dependencies.DownloadAssimp("master");
Util.Clone("https://github.com/g-truc/glm", "glm", "0.9.8");
Util.Clone("https://github.com/ocornut/imgui", "imgui", "docking");
Premake.Download("5.0.0-beta2");
Premake.GenerateSolution();















