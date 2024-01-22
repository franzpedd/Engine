#pragma once

#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26495 )
#endif
#include <tiny_gltf.h>
#if defined(_MSC_VER)
#pragma warning( pop)
#endif

#include <vulkan/vulkan.h>
#include <memory>

namespace Cosmos
{
	// forward declarations
	class Device;
	struct TextureSampler;

	class Texture2D
	{
	public:

		typedef enum class MSAA
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
		static std::shared_ptr<Texture2D> Create(std::shared_ptr<Device> device, const char* path, MSAA msaa = MSAA::SAMPLE_1_BIT, bool ktx = false);

		// creates a texture from an tinygltf image
		static std::shared_ptr<Texture2D> Create(std::shared_ptr<Device> device, tinygltf::Image& image, TextureSampler sampler, MSAA msaa = MSAA::SAMPLE_1_BIT);

		// constructor
		Texture2D() = default;

		// destructor
		virtual ~Texture2D() = default;

	public:

		// returns a reference to the image view
		virtual VkImageView GetView() = 0;

		// returns a reference to the image sampler
		virtual VkSampler& GetSampler() = 0;

	private:

	};

	struct TextureSampler
	{
		typedef enum class Filter
		{
			FILTER_NEAREST = 0,
			FILTER_LINEAR = 1
		} Filter;

		typedef enum class AddressMode
		{
			ADDRESS_MODE_REPEAT = 0,
			ADDRESS_MODE_MIRRORED_REPEAT = 1,
			ADDRESS_MODE_CLAMP_TO_EDGE = 2,
			ADDRESS_MODE_CLAMP_TO_BORDER = 3
		} AddressMode;

		// translates the wrap mode for the renderer api
		static AddressMode WrapMode(int32_t wrap);

		// translates the filter mode for the renderer api
		static Filter FilterMode(int32_t filter);

		Filter mag = Filter::FILTER_NEAREST;
		Filter min = Filter::FILTER_NEAREST;
		AddressMode u = AddressMode::ADDRESS_MODE_REPEAT;
		AddressMode v = AddressMode::ADDRESS_MODE_REPEAT;
		AddressMode w = AddressMode::ADDRESS_MODE_REPEAT;
	};
}