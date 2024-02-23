#pragma once

#include <Engine.h>

namespace Cosmos::tools
{
	class TerrainBrush
	{
	public:

		// constructor
		TerrainBrush(std::shared_ptr<Device>& device, std::string name, std::string path);

		// destructor
		~TerrainBrush();

	private:

		std::shared_ptr<Device>& mDevice;
		std::string mName;
		std::string mPath;
		std::shared_ptr<Texture2D> mTexture;
	};
}