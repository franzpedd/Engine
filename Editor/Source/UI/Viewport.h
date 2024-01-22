#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declaration
	class ModelBrowser;
	class TextureBrowser;

	class Viewport : public Widget
	{
	public:

		// constructor
		Viewport(std::shared_ptr<GUI>& ui, std::shared_ptr<Renderer>& renderer, Camera* camera, 
			TextureBrowser* textureBrowser, ModelBrowser* modelBrowser);

		// destructor
		virtual ~Viewport() = default;

		// returns a reference to the command entry
		inline std::shared_ptr<CommandEntry>& GetCommandEntry() { return mCommandEntry; }

		// returns the current viewport size
		inline ImVec2 GetSize() { return mCurrentSize; }

		// returns the viewport window lower boundaries
		inline ImVec2 GetWindowContentRegionMin() { return mContentRegionMin; }

		// returns the viewport window higher boundaries
		inline ImVec2 GetWindowContentRegionMax() { return mContentRegionMax; }

	public:

		// updates the ui element
		virtual void OnUpdate() override;

		// window was recently resized
		virtual void OnWindowResize() override;

		// freeing resources
		virtual void OnDestroy() override;

	private:

		// creates the command entry resources
		void CreateResources();

		// displays a side menu only on viewport view
		void DisplaySideMenu();

	private:

		std::shared_ptr<GUI>& mUI;
		std::shared_ptr<Renderer>& mRenderer;
		Camera* mCamera;
		TextureBrowser* mTextureBrowser;
		ModelBrowser* mModelBrowser;

		std::shared_ptr<CommandEntry> mCommandEntry;

		ImVec2 mCurrentSize;
		ImVec2 mContentRegionMin;
		ImVec2 mContentRegionMax;
		ImVec2 mViewportPosition;

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