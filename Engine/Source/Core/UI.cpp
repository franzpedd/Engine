#include "UI.h"

#include "Platform/Window.h"
#include "Renderer/Commander.h"
#include "Renderer/Vulkan/VKDevice.h"
#include "Renderer/Vulkan/VKInstance.h"
#include "Renderer/Vulkan/VKSwapchain.h"
#include "Renderer/Vulkan/VKUtility.h"
#include "Util/Logger.h"

// not using own implementation of GLFW + VULKAN for ImGUI
#if defined(_MSC_VER)
#pragma warning( push )
#pragma warning( disable : 26451 )
#endif
#include <imgui.h>
#include <backends/imgui_impl_glfw.cpp>
#include <backends/imgui_impl_vulkan.cpp>
#if defined(_MSC_VER)
# pragma warning(pop)
#endif

#include <array>
#include <vector>

namespace Cosmos
{
	UIElementStack::UIElementStack()
	{
	}

	UIElementStack::~UIElementStack()
	{
		for (UIElement* element : mElements)
		{
			element->OnDeletion();
			delete element;
		}
	}

	void UIElementStack::PushOver(UIElement* element)
	{
		element->OnCreation();
		mElements.emplace_back(element);
	}

	void UIElementStack::PopOver(UIElement* element)
	{
		auto it = std::find(mElements.begin() + mMiddlePos, mElements.end(), element);
		if (it != mElements.end())
		{
			element->OnDeletion();
			mElements.erase(it);
		}
	}

	void UIElementStack::Push(UIElement* element)
	{
		element->OnCreation();
		mElements.emplace(mElements.begin() + mMiddlePos, element);
		mMiddlePos++;
	}

	void UIElementStack::Pop(UIElement* element)
	{
		auto it = std::find(mElements.begin(), mElements.begin() + mMiddlePos, element);
		if (it != mElements.begin() + mMiddlePos)
		{
			element->OnDeletion();
			mElements.erase(it);
			mMiddlePos--;
		}
	}

	std::shared_ptr<UI> UI::Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain)
	{
		return std::make_shared<UI>(window, instance, device, swapchain);
	}

	UI::UI(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain)
		: mWindow(window), mInstance(instance), mDevice(device), mSwapchain(swapchain)
	{
		CreateResources();
		SetupConfiguration();

		mCommanderEntry = CommandEntry::Create(device);
	}

	UI::~UI()
	{
		for (auto framebuffer : mFramebuffers)
		{
			vkDestroyFramebuffer(mDevice->Device(), framebuffer, nullptr);
		}

		vkDestroyRenderPass(mDevice->Device(), mRenderPass, nullptr);

		vkFreeCommandBuffers(mDevice->Device(), mCommandPool, (uint32_t)mCommandBuffers.size(), mCommandBuffers.data());
		vkDestroyCommandPool(mDevice->Device(), mCommandPool, nullptr);

		vkDestroyDescriptorPool(mDevice->Device(), mDescriptorPool, nullptr);

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UI::NewFrame()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
	}

	void UI::OnUpdate()
	{
		// draw ui elements
		ImGui::ShowDemoWindow();

		// update all elements
		for (UIElement* element : mUIElements.Elements())
		{
			element->OnUpdate();
		}
	}

	void UI::EndFrame()
	{
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void UI::Draw(VkCommandBuffer cmd)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void UI::SetImageCount(uint32_t count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void UI::OnResize()
	{
		// recreate framebuffer based on image views
		{
			for (auto framebuffer : mFramebuffers)
			{
				vkDestroyFramebuffer(mDevice->Device(), framebuffer, nullptr);
			}

			mFramebuffers.resize(mSwapchain->ImageViews().size());

			for (size_t i = 0; i < mSwapchain->ImageViews().size(); i++)
			{
				VkImageView attachments[] = { mSwapchain->ImageViews()[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = mRenderPass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = mSwapchain->Extent().width;
				framebufferCI.height = mSwapchain->Extent().height;
				framebufferCI.layers = 1;
				LOG_ASSERT(vkCreateFramebuffer(mDevice->Device(), &framebufferCI, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}

		for (UIElement* element : mUIElements.Elements())
		{
			element->OnResize();
		}
	}

	void UI::SetupConfiguration()
	{
		// initial config
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		//if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports)
		//{
		//	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		//}

		io.Fonts->AddFontDefault();
		io.IniFilename = "ui.ini";

		ImGui::StyleColorsDark();
		SetupCustomStyle();

		// create descriptor pool
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolCI = {};
		poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCI.pNext = nullptr;
		poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolCI.maxSets = 1000 * IM_ARRAYSIZE(poolSizes);
		poolCI.poolSizeCount = (uint32_t)IM_ARRAYSIZE(poolSizes);
		poolCI.pPoolSizes = poolSizes;
		LOG_ASSERT(vkCreateDescriptorPool(mDevice->Device(), &poolCI, nullptr, &mDescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool for the User Interface");

		// glfw and vulkan initialization
		ImGui::CreateContext();
		ImGui_ImplGlfw_InitForVulkan(mWindow->NativeWindow(), true);

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = mInstance->Instance();
		initInfo.PhysicalDevice = mDevice->PhysicalDevice();
		initInfo.Device = mDevice->Device();
		initInfo.Queue = mDevice->GraphicsQueue();
		initInfo.DescriptorPool = mDescriptorPool;
		initInfo.MinImageCount = mSwapchain->ImageCount();
		initInfo.ImageCount = mSwapchain->ImageCount();
		initInfo.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		initInfo.Allocator = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, mRenderPass);

		LOG_TO_TERMINAL(Logger::Severity::Trace, "Check usage of ui render pass");

		// upload fonts
		VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice, mCommandPool);
		ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		EndSingleTimeCommand(mDevice, mCommandPool, cmdBuffer);

		// destroy used resources for uploading fonts
		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void UI::CreateResources()
	{
		// render pass
		{
			VkAttachmentDescription attachment = {};
			attachment.format = mSwapchain->SurfaceFormat().format;
			attachment.samples = VK_SAMPLE_COUNT_1_BIT;
			attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
			attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
			attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

			VkAttachmentReference colorAttachment = {};
			colorAttachment.attachment = 0;
			colorAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpass = {};
			subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpass.colorAttachmentCount = 1;
			subpass.pColorAttachments = &colorAttachment;

			VkSubpassDependency dependency = {};
			dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
			dependency.dstSubpass = 0;
			dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependency.srcAccessMask = 0;
			dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

			VkRenderPassCreateInfo info = {};
			info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			info.attachmentCount = 1;
			info.pAttachments = &attachment;
			info.subpassCount = 1;
			info.pSubpasses = &subpass;
			info.dependencyCount = 1;
			info.pDependencies = &dependency;
			LOG_ASSERT(vkCreateRenderPass(mDevice->Device(), &info, nullptr, &mRenderPass) == VK_SUCCESS, "Failed to create render pass");
		}

		// command pool
		{
			QueueFamilyIndices indices = mDevice->FindQueueFamilies(mDevice->PhysicalDevice(), mDevice->Surface());

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			LOG_ASSERT(vkCreateCommandPool(mDevice->Device(), &cmdPoolInfo, nullptr, &mCommandPool) == VK_SUCCESS, "Failed to create command pool");
		}

		// command buffers
		{
			mCommandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = mCommandPool;
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)mCommandBuffers.size();
			vkAllocateCommandBuffers(mDevice->Device(), &cmdBufferAllocInfo, mCommandBuffers.data());
		}

		// frame buffers
		{
			mFramebuffers.resize(mSwapchain->ImageViews().size());

			for (size_t i = 0; i < mSwapchain->ImageViews().size(); i++)
			{
				VkImageView attachments[] = { mSwapchain->ImageViews()[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = mRenderPass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = mSwapchain->Extent().width;
				framebufferCI.height = mSwapchain->Extent().height;
				framebufferCI.layers = 1;
				LOG_ASSERT(vkCreateFramebuffer(mDevice->Device(), &framebufferCI, nullptr, &mFramebuffers[i]) == VK_SUCCESS, "Failed to create framebuffer");
			}
		}
	}

	void UI::SetupCustomStyle()
	{
		ImGuiStyle& style = ImGui::GetStyle();

		// Color palette
		ImVec4 _black = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		ImVec4 _white = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		ImVec4 _grey = ImVec4(0.60f, 0.60f, 0.60f, 0.35f);
		ImVec4 _dark = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
		ImVec4 _darkgrey = ImVec4(0.23f, 0.23f, 0.23f, 0.35f);
		ImVec4 _lighgrey = ImVec4(0.40f, 0.40f, 0.40f, 1.00f);

		// Color array
		style.Colors[ImGuiCol_Text] = _white;
		style.Colors[ImGuiCol_TextDisabled] = _grey;
		style.Colors[ImGuiCol_WindowBg] = _dark;
		style.Colors[ImGuiCol_ChildBg] = _dark;
		style.Colors[ImGuiCol_PopupBg] = _dark;
		style.Colors[ImGuiCol_Border] = _grey;
		style.Colors[ImGuiCol_BorderShadow] = _black;
		style.Colors[ImGuiCol_FrameBg] = _darkgrey;
		style.Colors[ImGuiCol_FrameBgHovered] = _grey;
		style.Colors[ImGuiCol_FrameBgActive] = _grey;
		style.Colors[ImGuiCol_TitleBg] = _darkgrey;
		style.Colors[ImGuiCol_TitleBgActive] = _darkgrey;
		style.Colors[ImGuiCol_TitleBgCollapsed] = _darkgrey;
		style.Colors[ImGuiCol_MenuBarBg] = _darkgrey;
		style.Colors[ImGuiCol_ScrollbarBg] = _darkgrey;
		style.Colors[ImGuiCol_ScrollbarGrabHovered] = _grey;
		style.Colors[ImGuiCol_ScrollbarGrabActive] = _grey;
		style.Colors[ImGuiCol_CheckMark] = _lighgrey;
		style.Colors[ImGuiCol_SliderGrab] = _lighgrey;
		style.Colors[ImGuiCol_SliderGrabActive] = _white;
		style.Colors[ImGuiCol_Button] = _darkgrey;
		style.Colors[ImGuiCol_ButtonHovered] = _grey;
		style.Colors[ImGuiCol_ButtonActive] = _darkgrey;
		style.Colors[ImGuiCol_Header] = _darkgrey;
		style.Colors[ImGuiCol_HeaderHovered] = _grey;
		style.Colors[ImGuiCol_HeaderActive] = _grey;
		style.Colors[ImGuiCol_Separator] = _grey;
		style.Colors[ImGuiCol_SeparatorHovered] = _grey;
		style.Colors[ImGuiCol_SeparatorActive] = _grey;
		style.Colors[ImGuiCol_ResizeGrip] = _darkgrey;
		style.Colors[ImGuiCol_ResizeGripHovered] = _grey;
		style.Colors[ImGuiCol_ResizeGripActive] = _grey;
		style.Colors[ImGuiCol_Tab] = _darkgrey;
		style.Colors[ImGuiCol_TabHovered] = _grey;
		style.Colors[ImGuiCol_TabActive] = _grey;
		style.Colors[ImGuiCol_TabUnfocused] = _grey;
		style.Colors[ImGuiCol_TabUnfocused] = _grey;
		style.Colors[ImGuiCol_TabUnfocusedActive] = _grey;
		style.Colors[ImGuiCol_DockingPreview] = _grey;
		style.Colors[ImGuiCol_DockingEmptyBg] = _grey;
		style.Colors[ImGuiCol_PlotLines] = _white;
		style.Colors[ImGuiCol_PlotLinesHovered] = _grey;
		style.Colors[ImGuiCol_PlotHistogram] = _white;
		style.Colors[ImGuiCol_PlotHistogramHovered] = _grey;
		style.Colors[ImGuiCol_TableHeaderBg] = _dark;
		style.Colors[ImGuiCol_TableBorderStrong] = _darkgrey;
		style.Colors[ImGuiCol_TableBorderLight] = _grey;
		style.Colors[ImGuiCol_TableRowBg] = _black;
		style.Colors[ImGuiCol_TableRowBgAlt] = _white;
		style.Colors[ImGuiCol_TextSelectedBg] = _darkgrey;
		style.Colors[ImGuiCol_DragDropTarget] = _darkgrey;
		style.Colors[ImGuiCol_NavHighlight] = _grey;
		style.Colors[ImGuiCol_NavWindowingHighlight] = _grey;
		style.Colors[ImGuiCol_NavWindowingDimBg] = _grey;
		style.Colors[ImGuiCol_ModalWindowDimBg] = _grey;

		// Style
		style.FrameRounding = 3;
		style.WindowPadding = ImVec2(10.0f, 10.0f);
		style.FramePadding = ImVec2(10.00f, 10.00f);
		style.CellPadding = ImVec2(10.00f, 5.00f);
		style.ItemSpacing = ImVec2(10.00f, 5.00f);
		style.ItemInnerSpacing = ImVec2(5.00f, 5.00f);
		style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style.IndentSpacing = 15;
		style.ScrollbarSize = 18;
		style.GrabMinSize = 10;
		style.WindowBorderSize = 0;
		style.ChildBorderSize = 0;
		style.PopupBorderSize = 0;
		style.FrameBorderSize = 0;
		style.TabBorderSize = 0;
		style.WindowRounding = 5;
		style.ChildRounding = 5;
		style.PopupRounding = 0;
		style.ScrollbarRounding = 5;
		style.GrabRounding = 0;
		style.LogSliderDeadzone = 0;
		style.TabRounding = 0;
		style.WindowTitleAlign = ImVec2(0.50f, 0.50f);
		style.WindowMenuButtonPosition = -1;
		style.ColorButtonPosition = 0;
	}
}

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

	void RemoveTexture(VkDescriptorSet descriptorSet)
	{
		ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
		ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
		vkFreeDescriptorSets(v->Device, v->DescriptorPool, 1, &descriptorSet);
	}

	void Image(VkDescriptorSet image, Vec2& size)
	{
		ImGui::Image((ImTextureID)image, ImVec2{ size.x, size.y });
	}

	Vec2 GetContentRegionAvail()
	{
		return Vec2{ ImGui::GetContentRegionAvail().x , ImGui::GetContentRegionAvail().y };
	}
}