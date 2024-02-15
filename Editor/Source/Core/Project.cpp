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

	void Project::New(std::filesystem::path path)
	{
		//mScene->GetEntityMap().clear();
		//mScene->Registry().clear();
		//
		//SaveFileDialog dialog;
		//dialog.Show();
		//
		//if (dialog.GetFilePath().empty())
		//	return;
		//
		//mName = dialog.GetFileName();
		//mPath = dialog.GetFilePath();
		//
		//LOG_TO_TERMINAL(Logger::Trace, "Creating new project '%s'", mName.c_str());
	}

	void Project::Open()
	{
		LOG_TO_TERMINAL(Logger::Warn, "Save current Project must be implemented");

		// clean current project
		mScene->CleanCurrentScene();


		//mScene->GetEntityMap().clear();
		//mScene->Registry().clear();
		//
		//OpenFileDialog dialog;
		//dialog.Show();
		//
		//if (dialog.GetFilePath().empty())
		//	return;
		//
		//mPath = dialog.GetFilePath();
		//mName = mPath.filename().string();
		//
		//LOG_TO_TERMINAL(Logger::Trace, "Loading project '%s'", mName.c_str());
		//
		//DataFile project;
		//std::stringstream strbuff;
		//std::ifstream inputFile(mPath);
		//inputFile.open(mPath, std::ios::in);
		//
		//if (inputFile.is_open())
		//{
		//	strbuff << inputFile.rdbuf();
		//	//project = OrderedSerializer::parse(strbuff.str());
		//}
		//
		//inputFile.close();
		//
		//mScene->Load(project);
	}

	void Project::Save()
	{
		LOG_TO_TERMINAL(Logger::Trace, "Saving project '%s'", mName.c_str());

		std::filesystem::path savePath = mPath;
		savePath /= "Data";
		savePath /= mName;

		std::string savePathStr = savePath.string();
		savePathStr.append(".cosmos");

		DataFile::Write(mScene->Serialize(), savePathStr);
	}

	void Project::SaveAs()
	{
		//SaveFileDialog dialog;
		//dialog.Show();
		//
		//mName = dialog.GetFileName();
		//mPath = dialog.GetFilePath();
		//
		//DataFile save = mScene->Serialize();
		//
		//std::fstream outputFile;
		//outputFile.open(mPath, std::ios::out);
		//
		//if (outputFile.is_open())
		//{
		//	//outputFile << save.dump() << std::endl;
		//}
		//
		//outputFile.close();
	}
}