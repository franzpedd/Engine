#include "UI.h"

#include "Platform/Window.h"
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

#include <vector>

namespace Cosmos
{
	std::shared_ptr<UI> UI::Create(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain)
	{
		return std::make_shared<UI>(window, instance, device, swapchain);
	}

	UI::UI(std::shared_ptr<Window>& window, std::shared_ptr<VKInstance>& instance, std::shared_ptr<VKDevice>& device, std::shared_ptr<VKSwapchain>& swapchain)
		: mWindow(window), mInstance(instance), mDevice(device), mSwapchain(swapchain)
	{
		
	}

	UI::~UI()
	{
		vkDestroyDescriptorPool(mDevice->Device(), mDescriptorPool, nullptr);
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UI::Update()
	{
		// init new frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		// draw ui elements
		ImGui::ShowDemoWindow();

		// render
		ImGui::Render();
	}

	void UI::Draw(VkCommandBuffer cmd)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void UI::SetImageCount(uint32_t count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void UI::SetupConfiguration(VkRenderPass& renderPass)
	{
		// initial config
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
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
		poolCI.flags = 0;
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
		initInfo.MSAASamples = mDevice->GetMaxUsableSamples();
		initInfo.Allocator = nullptr;
		ImGui_ImplVulkan_Init(&initInfo, renderPass);

		LOG_TO_TERMINAL(Logger::Severity::Trace, "Check usage of ui render pass");

		// upload fonts
		VkCommandBuffer cmdBuffer = BeginSingleTimeCommand(mDevice);
		ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
		EndSingleTimeCommand(mDevice, cmdBuffer);

		// destroy used resources for uploading fonts
		ImGui_ImplVulkan_DestroyFontUploadObjects();
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