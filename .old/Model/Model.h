#pragma once

#include "Loader.h"
#include <memory>

namespace Cosmos
{
	// forward declaration
	class Device;

	class Model
	{
	public:

		// returns a smart-ptr to a new mesh
		static std::shared_ptr<Model> Create(std::shared_ptr<Renderer>& renderer);

		// constructor
		Model(std::shared_ptr<Renderer>& renderer);

		// destructor
		~Model();

	public:

		// return the model's path
		inline std::string GetPath() { return mPath; }

		// returns if the model is already loaded
		inline bool IsLoaded() const { return mLoaded; }

	public:

		// updates the animation
		void UpdateAnimations(float timeStep, uint32_t index);

		// draws the model
		void Draw(VkCommandBuffer commandBuffer);

		// loads a model from file path
		void LoadFromFile(std::string path);

		// free used resources
		void Destroy();

	private:

		// draws the node and it's children
		void DrawNode(VkCommandBuffer commandBuffer, ModelHelper::Node* node);

		// calculates the model's dimension
		void CalculateDimension();
		
		// calculates a single node bounding box
		void CalculateNodeBoundingBox(ModelHelper::Node* node, ModelHelper::Node* parent);

	private:

		std::shared_ptr<Renderer>& mRenderer;
		std::string mPath = {};
		ModelHelper::Loader* mLoader = nullptr;

		// model dimension
		glm::vec3 mDimensionMin = glm::vec3(FLT_MIN);
		glm::vec3 mDimensionMax = glm::vec3(FLT_MAX);
		glm::mat4 mAABB = {};

		// tells if any model was loaded into the class
		bool mLoaded = false;
	};
}