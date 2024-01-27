#pragma once

#include "Renderer/Texture.h"

namespace Cosmos
{
	class Skybox
	{
	public:

		// constructor
		Skybox(std::shared_ptr<Device>& device, std::array<std::string, 6> paths);

		// destructor
		~Skybox();

	private:

		// loads the cubemap
		void Load();

	private:

		std::shared_ptr<Device>& mDevice;
		std::array<std::string, 6> mPaths;
		std::array<std::shared_ptr<Texture2D>, 6> mTextures;
	};
}