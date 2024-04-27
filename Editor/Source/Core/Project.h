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
		Project(std::string name);

		// destructor
		~Project();

		// returns a reference to the project's name
		inline std::string& GetName() { return mName; }

		// returns a reference to the project's path
		inline std::string& GetPath() { return mPath; }

	public:

		// creates an empty project
		void New();

		// opens a project
		void Open();

		// saves current project
		void Save();

		// saves current project into another location
		void SaveAs();

	private:

		std::string mName = "Untitled";
		std::string mPath;
	};
}