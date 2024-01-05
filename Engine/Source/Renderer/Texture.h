#pragma once

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Device;

	class Texture2D
	{
	public:

		typedef enum MSAA
		{
			SAMPLE_1_BIT = 0x00000001,
			SAMPLE_2_BIT = 0x00000002,
			SAMPLE_4_BIT = 0x00000004,
			SAMPLE_8_BIT = 0x00000008,
			SAMPLE_16_BIT = 0x00000010,
			SAMPLE_32_BIT = 0x00000020,
			SAMPLE_64_BIT = 0x00000040,
		} MSAA;

	public:

		// creates a texture from an input file
		static std::shared_ptr<Texture2D> Create(std::shared_ptr<Device> device, const char* path, MSAA msaa = SAMPLE_1_BIT, bool ktx = false);

		// constructor
		Texture2D() = default;

		// destructor
		virtual ~Texture2D() = default;

	public:

		// returns a reference to the image view
		virtual VkImageView GetView() = 0;

		// returns a reference to the image sampler
		virtual VkSampler& GetSampler() = 0;
	};
}