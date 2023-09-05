#pragma once

#include "Util/Math.h"
#include <vulkan/vulkan.h>

namespace Cosmos::ui
{
	// starts the context for new window
	void Begin(const char* title);

	// ends the context for last bound window
	void End();

	// starts the child context of a window
	void BeginChild(const char* idStr);

	// ends the context for last bound child
	void EndChild();

	// returns the current ui cursor position, used for inserting next widget
	math::Vec2 GetCursorPos();

	// sets the new ui cursor positino, used for inserting next widget
	void SetCursonPos(const math::Vec2& pos);

	// adds a texture in the user interface for later usage
	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout);

	// shows an image to the bound ui window
	void Image(VkDescriptorSet image, math::Vec2& size);

	// enables the dockspace over the window drawable area
	void DockspaceOverEverything();

	// returns the window size of currently bound window
	math::Vec2 GetContentRegionAvail();

	// image button
	bool ButtonImage(const char* strId, VkDescriptorSet idTexture, const math::Vec2& size);

	// signals to put the next widget in the same line as the previous
	void SameLine();

	// signals to put the next widget in the next line from the previous
	void NextLine();

	// draws text with variadic arguments
	void Text(const char* fmt, ...);

	// creates a new group context
	void BeginGroup();

	// ends the group context
	void EndGroup();
}