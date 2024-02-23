#include "TerrainBrush.h"

namespace Cosmos::tools
{
	TerrainBrush::TerrainBrush(std::shared_ptr<Device>& device, std::string name, std::string path)
		: mDevice(device), mName(name), mPath(path)
	{

	}

	TerrainBrush::~TerrainBrush()
	{
	}
}