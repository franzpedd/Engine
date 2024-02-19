#include "epch.h"
#include "FileSystem.h"

#include "Util.h"

#include <filesystem>

namespace Cosmos::util
{
	std::string GetBinDir()
	{
		std::string binDir = std::filesystem::current_path().string();
		util::replace(binDir.begin(), binDir.end(), '\\', '/');

		return binDir;
	}

	std::string GetAssetDir()
	{
		// assuming the folder we're in is Editor or Game
		// this allows us to load stuff on editor or game relatively, it is hacky but allow
		// files to be saved without full path directories
		std::string assetsFolder = "../Data/";

		return assetsFolder;
	}

	std::string GetAssetSubDir(std::string subpath, bool removeExtension)
	{
		std::string assets = GetAssetDir();
		assets.append(subpath);

		if (removeExtension)
		{
			assets.erase(assets.begin() + assets.find_last_of('.'), assets.end());
		}

		return assets;
	}
}