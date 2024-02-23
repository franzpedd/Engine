#pragma once

#include "TerrainBrush.h"
#include <Engine.h>

namespace Cosmos::tools
{
	class TerrainGenerator : public Widget
	{
	public:

		// constructor
		TerrainGenerator(std::shared_ptr<Renderer> renderer, Scene* scene);

		// destructor
		~TerrainGenerator();

	public:

		// for user interface drawing
		virtual void OnUpdate() override;

	public:

		// displays or hides the window
		bool DisplayGeneratorMenu(bool value);

	private:

		// draws the generator menu
		void DrawGeneratorMenu();

		// create used resources for the terrain generator
		void CreateResources();

		// creates a mesh plane mesh
		void CreateTerrainPlane(int32_t width, int32_t height, int32_t divisions);

	private:

		std::shared_ptr<Renderer> mRenderer;
		Scene* mScene = nullptr;
		bool mDisplayGeneratorMenu = false;

		std::unordered_map<std::string, TerrainBrush> mBrushLibrary;
		TerrainBrush* mSelectedBrush = nullptr;

		// used to keep track of last desired values
		int32_t mWidth = 0;
		int32_t mHeight = 0;
		int32_t mDivisions = 0;
	};
}