#include "UIDrawable.h"

// not using own implementation of GLFW + VULKAN for ImGUI
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26451 )
#endif
#include <imgui.h>
#include <backends/imgui_impl_vulkan.cpp>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

namespace Cosmos::ui
{
	void Begin(const char* title)
	{
		ImGui::Begin(title);
	}

	void End()
	{
		ImGui::End();
	}

	void BeginChild(const char* idStr)
	{
		ImGui::BeginChild(idStr);
	}

	void EndChild()
	{
		ImGui::EndChild();
	}

	math::Vec2 GetCursorPos()
	{
		return { ImGui::GetCursorPos().x, ImGui::GetCursorPos().y };
	}

	void SetCursonPos(const math::Vec2& pos)
	{
		ImGui::SetCursorPos(ImVec2{ pos.x, pos.y });
	}

	VkDescriptorSet AddTexture(VkSampler sampler, VkImageView view, VkImageLayout layout)
	{
		ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
		ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;

		// create descriptor set
		VkDescriptorSet descriptorSet;
		{
			VkDescriptorSetAllocateInfo alloc_info = {};
			alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			alloc_info.descriptorPool = v->DescriptorPool;
			alloc_info.descriptorSetCount = 1;
			alloc_info.pSetLayouts = &bd->DescriptorSetLayout;
			VkResult err = vkAllocateDescriptorSets(v->Device, &alloc_info, &descriptorSet);
			check_vk_result(err);
		}

		// update descriptor set
		{
			VkDescriptorImageInfo desc_image[1] = {};
			desc_image[0].sampler = sampler;
			desc_image[0].imageView = view;
			desc_image[0].imageLayout = layout;

			VkWriteDescriptorSet write_desc[1] = {};
			write_desc[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write_desc[0].dstSet = descriptorSet;
			write_desc[0].descriptorCount = 1;
			write_desc[0].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write_desc[0].pImageInfo = desc_image;
			vkUpdateDescriptorSets(v->Device, 1, write_desc, 0, nullptr);
		}

		return descriptorSet;
	}

	void Image(VkDescriptorSet image, math::Vec2& size)
	{
		ImGui::Image((ImTextureID)image, ImVec2{ size.x, size.y });
	}

	void DockspaceOverEverything()
	{
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImGui::SetNextWindowPos(viewport->WorkPos);
		ImGui::SetNextWindowSize(viewport->WorkSize);

		const ImGuiWindowFlags wFlags = ImGuiWindowFlags_NoTitleBar
			| ImGuiWindowFlags_NoCollapse
			| ImGuiWindowFlags_NoResize
			| ImGuiWindowFlags_NoMove
			| ImGuiWindowFlags_NoBringToFrontOnFocus
			| ImGuiWindowFlags_NoNavFocus;

		ImGui::Begin("Dockspace", 0, wFlags);

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f));

		ImGui::End();

	}

	math::Vec2 GetContentRegionAvail()
	{
		return math::Vec2{ ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y };
	}

	bool ButtonImage(const char* strId, VkDescriptorSet idTexture, const math::Vec2& size)
	{
		return ImGui::ImageButton(strId, (ImTextureID)idTexture, { size.x, size.y });
	}

	void SameLine()
	{
		ImGui::SameLine();
	}

	void NextLine()
	{
		ImGui::NewLine();
	}

	void Text(const char* fmt, ...)
	{
		va_list args;
		va_start(args, fmt);
		ImGui::TextV(fmt, args);
		va_end(args);
	}

	void BeginGroup()
	{
		ImGui::BeginGroup();
	}

	void EndGroup()
	{
		ImGui::EndGroup();
	}
}