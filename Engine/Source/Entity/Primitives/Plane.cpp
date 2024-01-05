#include "epch.h"
#include "Plane.h"

#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Vulkan/VKShader.h"
#include "Renderer/Vulkan/VKPipeline.h"

namespace Cosmos
{
	Plane::Plane(Scene* scene, std::shared_ptr<Renderer>& renderer, Camera& camera)
		: Entity(scene), mScene(scene), mRenderer(renderer), mCamera(camera)
	{
		Logger() << "Creating Plane Primitive";
		LOG_TO_TERMINAL(Logger::Error, "Plane is not working");

		// setup initial config
		mVertices.resize(4);
		mIndices = { 0, 1, 2, 2, 3, 0 };

		// vertex position
		mVertices[0].position = { 0.0f, 0.0f, 0.1f };
		mVertices[1].position = { 1.0f, 0.0f, 0.1f };
		mVertices[2].position = { 1.0f, 1.0f, 0.1f };
		mVertices[3].position = { 0.0f, 1.0f, 0.1f };

		mVertices[0].color = { 1.0f, 1.0f, 1.0f };
		mVertices[1].color = { 1.0f, 1.0f, 0.0f };
		mVertices[2].color = { 1.0f, 0.0f, 1.0f };
		mVertices[3].color = { 0.0f, 1.0f, 1.0f };

		// resources
		CreatePipeline();
		CreateBuffers();
	}

	void Plane::OnRenderDraw()
	{
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

	void Plane::OnUpdate(float timestep)
	{
		VKGraphicsPipeline::UniformBufferObject ubo = {};
		ubo.model = glm::mat4(1.0f);
		ubo.view = glm::rotate(mCamera.GetView(), glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
		ubo.proj = mCamera.GetProjection();
		ubo.selected = mSelected;
		memcpy(mGraphicsPipeline->AccessUniformBuffers()[mRenderer->CurrentFrame()], &ubo, sizeof(ubo));
	}

	void Plane::OnDestroy()
	{
		mIndexBuffer->Destroy();
		mVertexBuffer->Destroy();
	}

	void Plane::CreatePipeline()
	{
		//// if already created just reference it
		//auto& graphicsLibrary = mRenderer->GetPipelineLibrary().AccessGraphicsTable();
		//if (graphicsLibrary.find("Plane") != graphicsLibrary.end())
		//{
		//	mGraphicsPipeline = mRenderer->GetPipelineLibrary().AccessGraphicsTable()["Plane"];
		//}
		//
		//else
		//{
		//	VKGraphicsPipeline::InitializerList graphicsPipeline(mRenderer->GetCommander().AccessMainCommandEntry()->renderPass, mRenderer->PipelineCache());
		//	graphicsPipeline.bindings = Vertex::GetBindingDescriptions();
		//	graphicsPipeline.attributes = Vertex::GetAttributeDescriptions();
		//	graphicsPipeline.vertexShader = VKShader::Create(mRenderer->GetDevice(), VKShader::ShaderType::Vertex, "Primitive Vert", "Data/Shaders/primitive.vert");
		//	graphicsPipeline.fragmentShader = VKShader::Create(mRenderer->GetDevice(), VKShader::ShaderType::Fragment, "Primitive Frag", "Data/Shaders/primitive.frag");
		//
		//	mGraphicsPipeline = VKGraphicsPipeline::Create(mRenderer->GetDevice(), graphicsPipeline);
		//
		//	mRenderer->GetPipelineLibrary().AccessGraphicsTable()["Plane"] = mGraphicsPipeline;
		//}
	}

	void Plane::CreateBuffers()
	{
		//mVertexBuffer = VKBuffer::Create
		//(
		//	mRenderer->GetDevice(),
		//	VKBuffer::Type::Vertex,
		//	sizeof(mVertices[0]) * mVertices.size(),
		//	mRenderer->GetCommander().AccessMainCommandEntry()->commandPool,
		//	mVertices.data()
		//);
		//
		//mIndexBuffer = VKBuffer::Create
		//(
		//	mRenderer->GetDevice(),
		//	VKBuffer::Type::Index,
		//	sizeof(mIndices[0]) * mIndices.size(),
		//	mRenderer->GetCommander().AccessMainCommandEntry()->commandPool,
		//	mIndices.data()
		//);
	}
}