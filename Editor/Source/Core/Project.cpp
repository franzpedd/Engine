#include "Project.h"

namespace Cosmos
{
	Project::Project(std::shared_ptr<Scene>& scene, std::filesystem::path path)
		: mScene(scene), mPath(path)
	{
	}

	Project::~Project()
	{
	}

	void Project::New(std::filesystem::path path)
	{
	}

	void Project::Open(std::filesystem::path path)
	{
	}

	void Project::Save()
	{
	}

	void Project::SaveAs(std::filesystem::path path)
	{
	}
}