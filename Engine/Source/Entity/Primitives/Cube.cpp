#include "epch.h"
#include "Cube.h"

#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"
#include "Renderer/Vulkan/VKPipeline.h"

namespace Cosmos
{
	Cube::Cube(Scene* scene, std::shared_ptr<Renderer>& renderer, Camera& camera)
		: Entity(scene), mScene(scene), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Primitive: Cube";

		// setup initial config
		mVertices.resize(8);
		mIndices =
		{
			0, 1, 2,	// t0 (front)
			2, 1, 3,	// t1 (front)
			4, 0, 6,	// t2 (left)
			6, 0, 2,	// t3 (left)
			7, 5, 6,	// t4 (back)
			6, 5, 4,	// t5 (back)
			3, 1, 7,	// t6 (right)
			7, 1, 5,	// t7 (right)
			4, 5, 0,	// t8 (under)
			0, 5, 1,	// t9 (under)
			3, 7, 2,	// t10 (top)
			2, 7, 6		// t11 (top)
		};

		// vertex position
		mVertices[0].position = { 0.0f, 0.0f, 0.0f }; // v0
		mVertices[1].position = { 1.0f, 0.0f, 0.0f }; // v1
		mVertices[2].position = { 0.0f, 1.0f, 0.0f }; // v2
		mVertices[3].position = { 1.0f, 1.0f, 0.0f }; // v3
		mVertices[4].position = { 0.0f, 0.0f, 1.0f }; // v4
		mVertices[5].position = { 1.0f, 0.0f, 1.0f }; // v5
		mVertices[6].position = { 0.0f, 1.0f, 1.0f }; // v6
		mVertices[7].position = { 1.0f, 1.0f, 1.0f }; // v7

		// color
		mVertices[0].color = { 1.0f, 0.0f, 0.0f };
		mVertices[1].color = { 0.0f, 1.0f, 0.0f };
		mVertices[2].color = { 0.0f, 0.0f, 1.0f };
		mVertices[3].color = { 1.0f, 0.0f, 0.0f };
		mVertices[4].color = { 0.0f, 1.0f, 0.0f };
		mVertices[5].color = { 0.0f, 0.0f, 1.0f };
		mVertices[6].color = { 0.0f, 1.0f, 0.0f };
		mVertices[7].color = { 0.0f, 0.0f, 1.0f };

		CreatePipeline();
		CreateBuffers();
	}

	void Cube::OnRenderDraw()
	{
		// pre-drawing
		VKGraphicsPipeline::UniformBufferObject ubo = {};
		ubo.model = glm::translate(ubo.model, glm::vec3(-1.0f, 1.0f, -1.0f));
		ubo.view = glm::rotate(mCamera.GetView(), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = mCamera.GetProjection();
		ubo.selected = true;
		memcpy(mGraphicsPipeline->AccessUniformBuffers()[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));

		// drawing
		VkDeviceSize vertexSize = mVertexBuffer->GetSize();
		VkDeviceSize indexSize = mIndexBuffer->GetSize();
		VkDeviceSize offsets[] = { 0 };
		auto& cmdBuffer = mRenderer->GetCommander().AccessMainCommandEntry()->commandBuffers[mRenderer->CurrentFrame()];

		vkCmdBindPipeline(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->GetPipeline());
		vkCmdBindVertexBuffers(cmdBuffer, 0, 1, &mVertexBuffer->Buffer(), offsets);
		vkCmdBindIndexBuffer(cmdBuffer, mIndexBuffer->Buffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(cmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, mGraphicsPipeline->GetPipelineLayout(), 0, 1, &mGraphicsPipeline->GetDescriptorSets()[mRenderer->CurrentFrame()], 0, nullptr);
		vkCmdDrawIndexed(cmdBuffer, uint32_t(mIndices.size()), 1, 0, 0, 0);
	}

	void Cube::OnUpdate(float timestep)
	{
	}

	void Cube::OnDestroy()
	{
		mIndexBuffer->Destroy();
		mVertexBuffer->Destroy();
	}

	void Cube::CreatePipeline()
	{
		// if already created just reference it
		auto& graphicsLibrary = mRenderer->GetPipelineLibrary().AccessGraphicsTable();
		if (graphicsLibrary.find("Cube") != graphicsLibrary.end())
		{
			mGraphicsPipeline = mRenderer->GetPipelineLibrary().AccessGraphicsTable()["Cube"];
		}

		else
		{
			VKGraphicsPipeline::InitializerList graphicsPipeline(mRenderer->GetCommander().AccessMainCommandEntry()->renderPass, mRenderer->PipelineCache());
			graphicsPipeline.bindings = Vertex::GetBindingDescriptions();
			graphicsPipeline.attributes = Vertex::GetAttributeDescriptions();
			graphicsPipeline.vertexShader = VKShader::Create(mRenderer->BackendDevice(), VKShader::ShaderType::Vertex, "Primitive Vert", "Data/Shaders/primitive.vert");
			graphicsPipeline.fragmentShader = VKShader::Create(mRenderer->BackendDevice(), VKShader::ShaderType::Fragment, "Primitive Frag", "Data/Shaders/primitive.frag");
			graphicsPipeline.cullMode = VK_CULL_MODE_FRONT_BIT;

			mGraphicsPipeline = VKGraphicsPipeline::Create(mRenderer->BackendDevice(), graphicsPipeline);

			mRenderer->GetPipelineLibrary().AccessGraphicsTable()["Cube"] = mGraphicsPipeline;
		}
	}

	void Cube::CreateBuffers()
	{
		mVertexBuffer = VKBuffer::Create
		(
			mRenderer->BackendDevice(),
			VKBuffer::Type::Vertex,
			sizeof(mVertices[0]) * mVertices.size(),
			mRenderer->GetCommander().AccessMainCommandEntry()->commandPool,
			mVertices.data()
		);
		
		mIndexBuffer = VKBuffer::Create
		(
			mRenderer->BackendDevice(),
			VKBuffer::Type::Index,
			sizeof(mIndices[0]) * mIndices.size(),
			mRenderer->GetCommander().AccessMainCommandEntry()->commandPool,
			mIndices.data()
		);
	}
}