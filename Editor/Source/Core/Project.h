#pragma once

#include <Engine.h>
#include <filesystem>
#include <memory>

namespace Cosmos
{
	class Project
	{
	public:

		// constructor
		Project(Scene* scene, std::string name);

		// destructor
		~Project();

		// returns a reference to the project's name
		inline std::string& GetName() { return mName; }

		// returns a reference to the project's path
		inline std::string& GetPath() { return mPath.string(); }

	public:

		// creates a new project
		void New(std::filesystem::path path);

		// opens a project
		void Open();

		// saves current project
		void Save();

		// saves current project into another location
		void SaveAs();

	private:

		Scene* mScene;
		std::string mName = "Untitled";
		std::filesystem::path mPath;
	};
}