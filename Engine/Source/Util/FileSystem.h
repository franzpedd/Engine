#pragma once

#include <string>
#include <vector>

namespace Cosmos
{
	// returns the binary directory path
	std::string GetBinDir();

	// returns the assets folder path
	std::string GetAssetDir();

	// returns the path of a sub-directory item that starts at the asset directory
	std::string GetAssetSubDir(std::string subpath, bool removeExtension = false);

	// reads a binary file and returns it's content
	std::vector<uint8_t> ReadFromBinary(std::string path);

	// writes the data into a binary file
	void WriteToBinary(std::string path, const void* data, size_t dataSize);
}