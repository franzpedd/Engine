#include "epch.h"
#include "Skybox.h"

namespace Cosmos
{
	Skybox::Skybox(std::shared_ptr<Device>& device, std::array<std::string, 6> paths)
		: mDevice(device), mPaths(paths)
	{
		Load();
	}

	Skybox::~Skybox()
	{
	}

	void Skybox::Load()
	{
		for (size_t i = 0; i < 6; i++)
		{
			mTextures[i] = Texture2D::Create(mDevice, mPaths[i].c_str());
		}
	}

}