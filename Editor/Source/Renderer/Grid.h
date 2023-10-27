#pragma once

#include <Engine.h>
#include <array>

namespace Cosmos
{
	// forward declarations
	class Viewport;

	class Grid : public Entity
	{
	public:

		struct UniformBufferObject
		{
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

	public:

		// constructor
		Grid(std::shared_ptr<Renderer>& renderer, Camera& camera);

		// destructor
		virtual ~Grid();

	public:

		// draws the entity (leave empty if doesnt required)
		virtual void OnDraw() override;

		// updates the entity (leave empty if doesnt required)
		virtual void OnUpdate(float timestep) override;

		// called before destructor, for freeing resources
		virtual void OnDrestroy() override;

	public:

		// toogles on/off if grid should be drawn
		void ToogleOnOff();

	private:

		// create all used resources
		void CreateResources();

	private:

		std::shared_ptr<Renderer>& mRenderer;
		Camera& mCamera;
		bool mVisible = true;
		
		std::shared_ptr<VKShader> mVertexShader;
		std::shared_ptr<VKShader> mFragmentShader;

		VkDescriptorSetLayout mDescriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool mDescriptorPool = VK_NULL_HANDLE;
		std::vector<VkDescriptorSet> mDescriptorSets;
		VkPipeline mGraphicsPipeline = VK_NULL_HANDLE;
		VkPipelineLayout mPipelineLayout = VK_NULL_HANDLE;

		std::vector<VkBuffer> mUniformBuffers;
		std::vector<VkDeviceMemory> mUniformBuffersMemory;
		std::vector<void*> mUniformBuffersMapped;
	};
}