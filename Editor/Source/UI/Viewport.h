#pragma once

#include <Engine.h>

namespace Cosmos
{
	// forward declaration
	class ModelBrowser;
	class TextureBrowser;
	class SceneHierarchy;

	class Viewport : public Widget
	{
	public:

		typedef enum GizmoType
		{
			UNDEFINED		= (0u << 0),
			TRANSLATE_X		= (1u << 0),
			TRANSLATE_Y		= (1u << 1),
			TRANSLATE_Z		= (1u << 2),
			ROTATE_X		= (1u << 3),
			ROTATE_Y		= (1u << 4),
			ROTATE_Z		= (1u << 5),
			ROTATE_SCREEN	= (1u << 6),
			SCALE_X			= (1u << 7),
			SCALE_Y			= (1u << 8),
			SCALE_Z			= (1u << 9),
			BOUNDS			= (1u << 10),
			SCALE_XU		= (1u << 11),
			SCALE_YU		= (1u << 12),
			SCALE_ZU		= (1u << 13),

			TRANSLATE		= TRANSLATE_X | TRANSLATE_Y | TRANSLATE_Z,
			ROTATE			= ROTATE_X | ROTATE_Y | ROTATE_Z | ROTATE_SCREEN,
			SCALE			= SCALE_X | SCALE_Y | SCALE_Z,
			SCALEU			= SCALE_XU | SCALE_YU | SCALE_ZU, // universal
			UNIVERSAL		= TRANSLATE | ROTATE | SCALEU

		} GizmoType;

	public:

		// constructor
		Viewport(std::shared_ptr<GUI>& ui, std::shared_ptr<Renderer>& renderer, std::shared_ptr<Camera>& camera, SceneHierarchy* sceneHierarcy,
			TextureBrowser* textureBrowser);

		// destructor
		virtual ~Viewport() = default;

		// returns a reference to the command entry
		inline std::shared_ptr<CommandEntry>& GetCommandEntry() { return mCommandEntry; }

		// returns the current viewport size
		inline ImVec2 GetSize() const { return mCurrentSize; }

		// returns the viewport window lower boundaries
		inline ImVec2 GetWindowContentRegionMin() const { return mContentRegionMin; }

		// returns the viewport window higher boundaries
		inline ImVec2 GetWindowContentRegionMax() const { return mContentRegionMax; }

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

		// draws the viewport menubar
		void DrawMenubar();

		// displays a side menu only on viewport view
		void DisplaySideMenu();

		// draws the gizmos on selected entity
		void DrawGizmos();

	private:

		std::shared_ptr<GUI>& mUI;
		std::shared_ptr<Renderer>& mRenderer;
		std::shared_ptr<Camera>& mCamera;
		SceneHierarchy* mSceneHierarcy;
		TextureBrowser* mTextureBrowser;

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

		GizmoType mGizmoType = GizmoType::UNDEFINED;
	};
}