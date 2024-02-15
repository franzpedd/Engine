#include "Project.h"

namespace Cosmos
{
	Project::Project(Scene* scene, std::string name)
		: mScene(scene), mName(name)
	{
	}

	Project::~Project()
	{
	}

	void Project::New()
	{
		LOG_TO_TERMINAL(Logger::Warn, "Save current Project must be implemented");
		
		// clean current project
		mScene->CleanCurrentScene();

		mName = "Untitled";
		mPath = std::filesystem::current_path();

		LOG_TO_TERMINAL(Logger::Warn, "New project '%s' created", mName);
	}

	void Project::Open()
	{
		LOG_TO_TERMINAL(Logger::Warn, "Save current Project must be implemented");
		
		// clean current project
		mScene->CleanCurrentScene();

		OpenFileDialog dialog;
		dialog.Show();

		if (dialog.GetFilePath().empty())
			return;

		mPath = dialog.GetFilePath();
		mName = dialog.GetFileName();

		LOG_TO_TERMINAL(Logger::Trace, "Loading project '%s'", mName.c_str());

		DataFile project;

		if (DataFile::Read(project, mPath.string()))
		{
			mScene->Deserialize(project);
		}
		
		else
		{
			LOG_TO_TERMINAL(Logger::Error, "Could not deserialize %s", mPath.string().c_str());
		}
	}

	void Project::Save()
	{
		LOG_TO_TERMINAL(Logger::Trace, "Saving project '%s'", mName.c_str());

		std::filesystem::path savePath = mPath;
		savePath /= "Data";
		savePath /= mName;

		std::string savePathStr = savePath.string();
		savePathStr.append(".cosmos");

		if (DataFile::Write(mScene->Serialize(), savePathStr))
		{
			LOG_TO_TERMINAL(Logger::Trace, "Project '%s' saved", mName.c_str());
		}

		else
		{
			LOG_TO_TERMINAL(Logger::Trace, "Error while saving '%s'", mName.c_str());
		}
	}

	void Project::SaveAs()
	{
		SaveFileDialog dialog;
		dialog.Show();
		
		mName = dialog.GetFileName();
		mPath = dialog.GetFilePath();

		LOG_TO_TERMINAL(Logger::Trace, "Saving Project '%s'", mName.c_str());

		if (DataFile::Write(mScene->Serialize(), mPath.string()))
		{
			LOG_TO_TERMINAL(Logger::Trace, "Project '%s' saved", mName.c_str());
		}

		else
		{
			LOG_TO_TERMINAL(Logger::Trace, "Error while saving '%s'", mName.c_str());
		}
	}
}