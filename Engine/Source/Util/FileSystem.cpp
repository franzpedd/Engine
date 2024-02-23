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

	std::vector<uint8_t> ReadFromBinary(std::string path)
	{
		std::ifstream file(path, std::fstream::in | std::fstream::binary);
		std::streampos size;

		if (file.is_open())
		{
			// retrieve bianary size
			file.seekg(0, std::ios::end);
			size = file.tellg();
			file.seekg(0, std::ios::beg);

			// read data
			std::vector<uint8_t> data(size);
			file.read((char*)&data[0], size);

			file.close();

			return data;
		}

		Logger() << "Error when opening file " << path << "for reading";
		return std::vector<uint8_t>();
	}

	void WriteToBinary(std::string path, const void* data, size_t dataSize)
	{
		std::ofstream file(path, std::fstream::out | std::fstream::binary | std::ios::app);
		
		if (file.is_open())
		{
			file.write(reinterpret_cast<char*>(&data), sizeof(dataSize));
			return;
		}

		Logger() << "Error when opening file " << path << "for writting";
		return;
	}
}