#include "epch.h"
#include "Model.h"

#include "Renderer/Renderer.h"
#include "Node.h"

namespace Cosmos
{
	std::shared_ptr<Model> Model::Create(std::shared_ptr<Renderer>& renderer)
	{
		return std::make_shared<Model>(renderer);
	}

	Model::Model(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{

	}

	void Model::UpdateAnimations(float timeStep, uint32_t index)
	{
		if (mLoader->animations.empty())
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "This model does not have animations");
			return;
		}
		
		if (index > (uint32_t)mLoader->animations.size() - 1)
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "There's no animation with index %d", index);
			return;
		}
		
		ModelHelper::Animation& animation = mLoader->animations[index];
		bool updated = false;
		
		for (auto& channel : animation.channels)
		{
			ModelHelper::Animation::Sampler& sampler = animation.samplers[channel.samplerIndex];
		
			if (sampler.inputs.size() > sampler.outputs.size())
				continue;
		
			for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
			{
				if ((timeStep >= sampler.inputs[i]) && (timeStep <= sampler.inputs[i + 1]))
				{
					float u = std::max(0.0f, timeStep - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);
		
					if (u <= 1.0f)
					{
						switch (channel.pathType)
						{
							case ModelHelper::Animation::Channel::PathType::TRANSLATION:
							{
								glm::vec4 trans = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], u);
								channel.node->translation = glm::vec3(trans);
								break;
							}
		
							case ModelHelper::Animation::Channel::PathType::SCALE:
							{
								glm::vec4 trans = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], u);
								channel.node->scale = glm::vec3(trans);
								break;
							}
		
							case ModelHelper::Animation::Channel::PathType::ROTATION:
							{
								glm::quat q1;
								q1.x = sampler.outputs[i].x;
								q1.y = sampler.outputs[i].y;
								q1.z = sampler.outputs[i].z;
								q1.w = sampler.outputs[i].w;
		
								glm::quat q2;
								q2.x = sampler.outputs[i + 1].x;
								q2.y = sampler.outputs[i + 1].y;
								q2.z = sampler.outputs[i + 1].z;
								q2.w = sampler.outputs[i + 1].w;
		
								channel.node->rotation = glm::normalize(glm::slerp(q1, q2, u));
								break;
							}
						}
						updated = true;
					}
				}
			}
		}
		
		if (updated)
			for (auto& node : mLoader->nodes)
				node->Update();
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		const VkDeviceSize offsets[1] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mLoader->vertexBuffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mLoader->indexBuffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& node : mLoader->nodes)
			DrawNode(commandBuffer, node);
	}

	void Model::LoadFromFile(std::string path)
	{
		mPath = path;
		mLoader = std::make_unique<ModelHelper::Loader>(mRenderer, mPath);

		CalculateDimension();
	}

	void Model::Destroy()
	{
		mLoader->Destroy();
	}

	void Model::DrawNode(VkCommandBuffer commandBuffer, ModelHelper::Node* node)
	{
		if (node->mesh != nullptr)
			for (ModelHelper::Primitive* primitive : node->mesh->primitives)
				vkCmdDrawIndexed(commandBuffer, primitive->indexCount, 1, primitive->firstIndex, 0, 0);

		for (auto& child : node->children)
			DrawNode(commandBuffer, child);
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
		BoundingBox parentBvh = parent ? parent->bvh : BoundingBox(mDimensionMin, mDimensionMax);
		
		if (node->mesh != nullptr)
		{
			if (node->mesh->bb.IsValid())
			{
				node->aabb = node->mesh->bb.AABB(node->matrix);
		
				if (node->children.size() == 0)
				{
					node->bvh.SetMin(node->aabb.GetMin());
					node->bvh.SetMax(node->aabb.GetMax());
					node->bvh.SetValid(true);
				}
			}
		}
		
		parentBvh.SetMin(glm::min(parentBvh.GetMin(), node->bvh.GetMin()));
		parentBvh.SetMax(glm::min(parentBvh.GetMax(), node->bvh.GetMax()));
		
		for (auto& child : node->children)
			CalculateNodeBoundingBox(child, node);
	}
}