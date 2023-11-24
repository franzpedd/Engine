#pragma once

#include <Engine.h>
#include <filesystem>

namespace Cosmos
{
	class Project
	{
	public:

		// constructor
		Project(std::shared_ptr<Scene>& scene, std::filesystem::path path);

		// destructor
		~Project();

	public:

		// creates a new project
		void New(std::filesystem::path path);

		// opens a project
		void Open(std::filesystem::path path);

		// saves current project
		void Save();

		// saves current project into another location
		void SaveAs(std::filesystem::path path);

	private:

		std::shared_ptr<Scene>& mScene;
		std::filesystem::path mPath;
	};
}