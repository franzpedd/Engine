#pragma once

#include <Engine.h>

namespace Cosmos
{
	class Viewport : public UIElement
	{
	public:

		// constructor
		Viewport(std::shared_ptr<UICore>& ui, std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		virtual ~Viewport();

		// returns a reference to the command entry
		inline std::shared_ptr<CommandEntry>& GetCommandEntry() { return mCommandEntry; }

		// returns the current viewport size
		inline ImVec2 GetSize() { return mCurrentSize; }

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
		Camera& mCamera;

		std::shared_ptr<CommandEntry> mCommandEntry;
		ImVec2 mCurrentSize;

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