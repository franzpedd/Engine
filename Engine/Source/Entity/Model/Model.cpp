#include "epch.h"
#include "Model.h"

#include "Renderer/Device.h"
#include "Node.h"

namespace Cosmos
{
	std::shared_ptr<Model> Model::Create(std::shared_ptr<Device>& device)
	{
		return std::make_shared<Model>(device);
	}

	Model::Model(std::shared_ptr<Device>& device)
		: mDevice(device)
	{
		
	}

	Model::~Model()
	{

	}

	void Model::LoadFromFile(std::string path)
	{
		mPath = path;
		mLoader = std::make_unique<ModelHelper::Loader>(mDevice, mPath);

		CalculateDimension();
	}

	void Model::Destroy()
	{
		mLoader->Destroy();
	}

	void Model::CalculateDimension()
	{
		// calculate binary volume hierarchy for all nodes in the scene/model
		for (auto node : mLoader->linearNodes)
			CalculateNodeBoundingBox(node, nullptr);
		
		mDimensionMin = glm::vec3(FLT_MAX);
		mDimensionMax = glm::vec3(-FLT_MAX);
		
		for (auto node : mLoader->linearNodes)
		{
			if (node->bvh.IsValid())
			{
				mDimensionMin = glm::min(mDimensionMin, node->bvh.GetMin());
				mDimensionMax = glm::max(mDimensionMax, node->bvh.GetMax());
			}
		}
		
		// calculate scene aabb
		mAABB = glm::scale(glm::mat4(1.0f), glm::vec3(mDimensionMax[0] - mDimensionMin[0], mDimensionMax[1] - mDimensionMin[1], mDimensionMax[2] - mDimensionMin[2]));
		mAABB[3][0] = mDimensionMin[0];
		mAABB[3][1] = mDimensionMin[1];
		mAABB[3][2] = mDimensionMin[2];
	}

	void Model::CalculateNodeBoundingBox(ModelHelper::Node* node, ModelHelper::Node* parent)
	{
	}
}