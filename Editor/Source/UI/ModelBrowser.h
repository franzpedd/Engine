#pragma once

#include <Engine.h>

namespace Cosmos
{
	class ModelBrowser : public Widget
	{
	public:

		enum Extension
		{
			UNDEFINED = -1,
			GLTF,

			EXTENSION_MAX
		};

	public:

		// constructure
		ModelBrowser(std::shared_ptr<Renderer>& renderer);

		// destructor
		~ModelBrowser();

		// for user interface drawing
		virtual void OnUpdate() override;

	public:

		// opens/closes the widget
		void ToogleOnOff(bool value);

		// returns a copy of the last selected model in the texture browser
		inline std::string GetSelectedModel() { return mLastModelSelected; }

	private:

		// searches for available models
		void RefreshFilesExplorer(std::string path);

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::string mRoot = "Data";
		bool mShowWindow = false;
		std::string mSearchStr;
		bool mUpdatedStr = false;
		const std::array<const char*, EXTENSION_MAX> mValidExtensions = { ".gltf" };
		std::vector<std::string> mModels;
		std::string mLastModelSelected = {};
	};
}