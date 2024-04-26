#include "Project.h"

namespace Cosmos
{
	Project::Project(std::string name)
		: mName(name)
	{
		mPath = GetAssetDir();
	}

	Project::~Project()
	{
	}

	void Project::New()
	{
		LOG_TO_TERMINAL(Logger::Warn, "Save current Project must be implemented");
		
		// clean current project
		//Application::GetInstance()->GetActiveScene()->CleanCurrentScene();

		mName = "Untitled";
		mPath = GetAssetDir();

		LOG_TO_TERMINAL(Logger::Warn, "New project '%s' created", mName);
	}

	void Project::Open()
	{
		LOG_TO_TERMINAL(Logger::Warn, "Save current Project must be implemented");
		
		// clean current project
		//Application::GetInstance()->GetActiveScene()->CleanCurrentScene();

		OpenFileDialog dialog;
		dialog.Show();

		if (dialog.GetFilePath().empty())
			return;

		mPath = dialog.GetFilePath();
		mName = dialog.GetFileName();

		LOG_TO_TERMINAL(Logger::Trace, "Loading project '%s'", mName.c_str());

		DataFile project;

		if (DataFile::Read(project, mPath))
		{
			Application::GetInstance()->GetActiveScene()->Deserialize(project);
		}
		
		else
		{
			LOG_TO_TERMINAL(Logger::Error, "Could not deserialize %s", mPath.c_str());
		}
	}

	void Project::Save()
	{
		LOG_TO_TERMINAL(Logger::Trace, "Saving project '%s'", mName.c_str());

		std::string savePath = mPath;
		savePath.append(mName);
		savePath.append(".cosmos");

		if (DataFile::Write(Application::GetInstance()->GetActiveScene()->Serialize(), savePath))
		{
			LOG_TO_TERMINAL(Logger::Trace, "Project '%s' saved at '%s'", mName.c_str(), savePath.c_str());
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

		if (DataFile::Write(Application::GetInstance()->GetActiveScene()->Serialize(), mPath))
		{
			LOG_TO_TERMINAL(Logger::Trace, "Project '%s' saved at '%s' ", mName.c_str(), mPath.c_str());
		}

		else
		{
			LOG_TO_TERMINAL(Logger::Trace, "Error while saving '%s'", mName.c_str());
		}
	}
}