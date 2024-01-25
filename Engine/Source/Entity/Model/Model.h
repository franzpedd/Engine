#pragma once

#include "Loader.h"
#include <memory>

namespace Cosmos
{
	class Model
	{
	public:

		// returns a smart-ptr to a new mesh
		static std::shared_ptr<Model> Create(std::shared_ptr<Device>& device);

		// constructor
		Model(std::shared_ptr<Device>& device);

		// destructor
		~Model();

	public:

		// return the model's path
		inline std::string GetPath() { return mPath; }

	public:

		// loads a model from file path
		void LoadFromFile(std::string path);

		// free used resources
		void Destroy();

	private:

		// calculates the model's dimension
		void CalculateDimension();
		
		// calculates a single node bounding box
		void CalculateNodeBoundingBox(ModelHelper::Node* node, ModelHelper::Node* parent);

	private:

		std::shared_ptr<Device>& mDevice;
		std::string mPath = {};
		std::unique_ptr<ModelHelper::Loader> mLoader;

		// model dimension
		glm::vec3 mDimensionMin = glm::vec3(FLT_MIN);
		glm::vec3 mDimensionMax = glm::vec3(FLT_MAX);
		glm::mat4 mAABB = {};
	};
}