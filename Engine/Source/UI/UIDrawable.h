#pragma once

#include "Util/Math.h"
#include <vulkan/vulkan.h>

namespace Cosmos::ui
{
	// starts the context for new window
	void Begin(const char* title);

	// ends the context for last bound window
	void End();

	// adds a texture in the user interface for later usage
	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout);

	// removes a texture from the user interface pool
	void RemoveTexture(VkDescriptorSet descriptorSet);

	// shows an image to the bound ui window
	void Image(VkDescriptorSet image, Vec2& size);

	// returns the window size of currently bound window
	Vec2 GetContentRegionAvail();
}