#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Viewport : public UIElement
	{
	public:

		// constructor
		Viewport(std::shared_ptr<UICore>& ui, std::shared_ptr<Renderer>& renderer);

		// destructor
		virtual ~Viewport();

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnResize() override;

	private:

		// creates the command entry resources
		void CreateResources();

	private:

		std::shared_ptr<UICore>& mUI;
		std::shared_ptr<Renderer>& mRenderer;
		std::shared_ptr<CommandEntry> mCommandEntry;

		VkSampler mSampler = VK_NULL_HANDLE;

		VkImage mDepthImage = VK_NULL_HANDLE;;
		VkDeviceMemory mDepthMemory = VK_NULL_HANDLE;
		VkImageView mDepthView = VK_NULL_HANDLE;

		VkFormat mSurfaceFormat = VK_FORMAT_UNDEFINED;
		VkFormat mDepthFormat = VK_FORMAT_UNDEFINED;

		std::vector<VkImage> mImages;
		std::vector<VkDeviceMemory> mImageMemories;
		std::vector<VkImageView> mImageViews;

		std::vector<VkDescriptorSet> mDescriptorSets;
	};
}