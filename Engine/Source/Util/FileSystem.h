#pragma once

#include <string>

namespace Cosmos::util
{
	// returns the binary directory path
	std::string GetBinDir();

	// returns the assets folder path
	std::string GetAssetDir();

	// returns the path of a sub-directory item that starts at the asset directory
	std::string GetAssetSubDir(std::string subpath, bool removeExtension = false);
}