#include "epch.h"
#include "GUI.h"

#include "Icons.h"
#include "Spectrum.h"
#include "Core/Application.h"
#include "Event/WindowEvent.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Renderer/Renderer.h"
#include "Util/FileSystem.h"

// Thirdparty/imgui_extra
#include <imguizmo.cpp>
#include <imgui_impl_sdl2.cpp>
#include <imgui_impl_vulkan.cpp>

#include <array>
#include <vector>

namespace Cosmos
{
	GUI* GUI::sGUI = nullptr;

	GUI::GUI(std::shared_ptr<Renderer> renderer)
		: mRenderer(renderer)
	{
		LOG_ASSERT(sGUI == nullptr, "GUI already created");
		sGUI = this;

		Commander::Get().Insert("ImGui");
		Commander::Get().GetEntries()["ImGui"]->msaa = VK_SAMPLE_COUNT_1_BIT;

		CreateResources();
		SetupConfiguration();
	}

	GUI::~GUI()
	{
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplSDL2_Shutdown();
		ImGui::DestroyContext();

		Commander::Get().Erase("ImGui", mRenderer->GetDevice()->GetDevice());
	}

	void GUI::OnUpdate()
	{
		PROFILER_FUNCTION();

		// new frame
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplSDL2_NewFrame();
		ImGui::NewFrame();
		ImGuizmo::BeginFrame();

		for (Widget* widget : mWidgetStack)
		{
			widget->OnUpdate();
		}

		// end frame
		ImGui::Render();

		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void GUI::OnRenderDraw()
	{
		for (Widget* widget : mWidgetStack)
		{
			widget->OnRender();
		}
	}

	void GUI::OnEvent(Shared<Event> event)
	{
		if (event->GetType() == EventType::WindowResize)
		{
			vkDeviceWaitIdle(mRenderer->GetDevice()->GetDevice());

			// recreate frame buffers
			for (auto framebuffer : Commander::Get().GetEntries()["ImGui"]->framebuffers)
			{
				vkDestroyFramebuffer(mRenderer->GetDevice()->GetDevice(), framebuffer, nullptr);
			}

			Commander::Get().GetEntries()["ImGui"]->framebuffers.resize(mRenderer->GetSwapchain()->GetImageViews().size());

			for (size_t i = 0; i < mRenderer->GetSwapchain()->GetImageViews().size(); i++)
			{
				VkImageView attachments[] = { mRenderer->GetSwapchain()->GetImageViews()[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = Commander::Get().GetEntries()["ImGui"]->renderPass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = mRenderer->GetSwapchain()->GetExtent().width;
				framebufferCI.height = mRenderer->GetSwapchain()->GetExtent().height;
				framebufferCI.layers = 1;

				VK_ASSERT
				(
					vkCreateFramebuffer
					(
						mRenderer->GetDevice()->GetDevice(),
						&framebufferCI,
						nullptr,
						&Commander::Get().GetEntries()["ImGui"]->framebuffers[i]
					),
					"Failed to create framebuffer"
				);
			}
		}

		for (Widget* widget : mWidgetStack)
		{
			widget->OnEvent(event);
		}
	}

	void GUI::Draw(VkCommandBuffer cmd)
	{
		ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), cmd);
	}

	void GUI::SetImageCount(uint32_t count)
	{
		ImGui_ImplVulkan_SetMinImageCount(count);
	}

	void GUI::SetupConfiguration()
	{
		// initial config
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		if (io.BackendFlags | ImGuiBackendFlags_PlatformHasViewports && io.BackendFlags | ImGuiBackendFlags_RendererHasViewports)
		{
			io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
		}

		static const ImWchar iconRanges1[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
		static const ImWchar iconRanges2[] = { ICON_MIN_LC, ICON_MAX_LC, 0 };
		constexpr float iconSize = 13.0f;
		constexpr float fontSize = 18.0f;

		ImFontConfig iconCFG;
		iconCFG.MergeMode = true;
		iconCFG.GlyphMinAdvanceX = iconSize;
		iconCFG.PixelSnapH = true;

		io.Fonts->Clear();
		LoadFont(fontSize);

		mFonts.iconFA = io.Fonts->AddFontFromFileTTF(util::GetAssetSubDir("Fonts/fontawesome-webfont.ttf").c_str(), iconSize, &iconCFG, iconRanges1);
		mFonts.iconLC = io.Fonts->AddFontFromFileTTF(util::GetAssetSubDir("Fonts/lucide.ttf").c_str(), iconSize, &iconCFG, iconRanges2);
		io.Fonts->Build();

		io.IniFilename = "ui.ini";
		io.WantCaptureMouse = true;

		ImGui::StyleColorsDark();
		StyleColorsSpectrum();

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
		VK_ASSERT(vkCreateDescriptorPool(mRenderer->GetDevice()->GetDevice(), &poolCI, nullptr, &Commander::Get().GetEntries()["ImGui"]->descriptorPool), "Failed to create descriptor pool for the User Interface");

		// glfw and vulkan initialization
		ImGui::CreateContext();
		ImGui_ImplSDL2_InitForVulkan(Application::GetInstance()->GetWindow()->GetNativeWindow());

		ImGui_ImplVulkan_InitInfo initInfo = {};
		initInfo.Instance = mRenderer->GetInstance()->GetInstance();
		initInfo.PhysicalDevice = mRenderer->GetDevice()->GetPhysicalDevice();
		initInfo.Device = mRenderer->GetDevice()->GetDevice();
		initInfo.Queue = mRenderer->GetDevice()->GetGraphicsQueue();
		initInfo.DescriptorPool = Commander::Get().GetEntries()["ImGui"]->descriptorPool;
		initInfo.MinImageCount = mRenderer->GetSwapchain()->GetImageCount();
		initInfo.ImageCount = mRenderer->GetSwapchain()->GetImageCount();
		initInfo.MSAASamples = Commander::Get().GetEntries()["ImGui"]->msaa;
		initInfo.Allocator = nullptr;
		initInfo.RenderPass = Commander::Get().GetEntries()["ImGui"]->renderPass;
		ImGui_ImplVulkan_Init(&initInfo);

		LOG_TO_TERMINAL(Logger::Severity::Trace, "Check usage of ui render pass");

		// upload fonts
		ImGui_ImplVulkan_CreateFontsTexture();
	}

	void GUI::CreateResources()
	{
		// render pass
		{
			VkAttachmentDescription attachment = {};
			attachment.format = mRenderer->GetSwapchain()->GetSurfaceFormat().format;
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
			VK_ASSERT(vkCreateRenderPass(mRenderer->GetDevice()->GetDevice(), &info, nullptr, &Commander::Get().GetEntries()["ImGui"]->renderPass), "Failed to create render pass");
		}

		// command pool
		{
			QueueFamilyIndices indices = mRenderer->GetDevice()->FindQueueFamilies(mRenderer->GetDevice()->GetPhysicalDevice(), mRenderer->GetDevice()->GetSurface());

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_ASSERT(vkCreateCommandPool(mRenderer->GetDevice()->GetDevice(), &cmdPoolInfo, nullptr, &Commander::Get().GetEntries()["ImGui"]->commandPool), "Failed to create command pool");
		}

		// command buffers
		{
			Commander::Get().GetEntries()["ImGui"]->commandBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = Commander::Get().GetEntries()["ImGui"]->commandPool;
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)Commander::Get().GetEntries()["ImGui"]->commandBuffers.size();
			VK_ASSERT(vkAllocateCommandBuffers(mRenderer->GetDevice()->GetDevice(), &cmdBufferAllocInfo, Commander::Get().GetEntries()["ImGui"]->commandBuffers.data()), "Failed to allocate command buffers");
		}

		// frame buffers
		{
			Commander::Get().GetEntries()["ImGui"]->framebuffers.resize(mRenderer->GetSwapchain()->GetImageViews().size());

			for (size_t i = 0; i < mRenderer->GetSwapchain()->GetImageViews().size(); i++)
			{
				VkImageView attachments[] = { mRenderer->GetSwapchain()->GetImageViews()[i] };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = Commander::Get().GetEntries()["ImGui"]->renderPass;
				framebufferCI.attachmentCount = 1;
				framebufferCI.pAttachments = attachments;
				framebufferCI.width = mRenderer->GetSwapchain()->GetExtent().width;
				framebufferCI.height = mRenderer->GetSwapchain()->GetExtent().height;
				framebufferCI.layers = 1;
				VK_ASSERT(vkCreateFramebuffer(mRenderer->GetDevice()->GetDevice(), &framebufferCI, nullptr, &Commander::Get().GetEntries()["ImGui"]->framebuffers[i]), "Failed to create framebuffer");
			}
		}
	}

	void GUI::HandleInternalEvent(SDL_Event* e)
	{
		ImGui_ImplSDL2_ProcessEvent(e);
	}

	void UIToggleCursor(bool hide)
	{
		ImGuiIO& io = ImGui::GetIO();

		if (hide)
		{
			io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;
			return;
		}

		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
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

	void RemoveTexture(VkDescriptorSet descriptor)
	{
		ImGui_ImplVulkan_Data* bd = ImGui_ImplVulkan_GetBackendData();
		ImGui_ImplVulkan_InitInfo* v = &bd->VulkanInitInfo;
		vkDeviceWaitIdle(v->Device);
		vkFreeDescriptorSets(v->Device, v->DescriptorPool, 1, &descriptor);
	}

	void ToogleMouseCursor(bool hide)
	{
		if (hide)
		{
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags |= ImGuiConfigFlags_NoMouse;

			return;
		}

		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags ^= ImGuiConfigFlags_NoMouse;
	}

	bool CheckboxEx(const char* label, bool* v)
	{
		ImGuiWindow* window = ImGui::GetCurrentWindow();
		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = g.Style;
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.y * 0.5f, label_size.y + style.FramePadding.y * 0.5f));

		ImGui::ItemSize(check_bb, style.FramePadding.y);

		ImRect total_bb = check_bb;
		if (label_size.x > 0) ImGui::SameLine(0, style.ItemInnerSpacing.x);

		const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y) - ImVec2(0, 2), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);

		if (label_size.x > 0)
		{
			ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
			total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
		}

		if (!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		if (pressed) *v = !(*v);

		ImGui::RenderFrame(check_bb.Min, check_bb.Max, ImGui::GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);
		if (*v)
		{
			const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
			const float pad = ImMax(1.0f, (float)(int)(check_sz / 6.0f));
			const ImVec2 pts[] =
			{
				ImVec2 {check_bb.Min.x + pad, check_bb.Min.y + ((check_bb.Max.y - check_bb.Min.y) / 2)},
				ImVec2 {check_bb.Min.x + ((check_bb.Max.x - check_bb.Min.x) / 3), check_bb.Max.y - pad * 1.5f},
				ImVec2 {check_bb.Max.x - pad, check_bb.Min.y + pad}
			};

			window->DrawList->AddPolyline(pts, 3, ImGui::GetColorU32(ImGuiCol_CheckMark), false, 2.0f);

			if (g.LogEnabled) ImGui::LogRenderedText(&text_bb.GetTL(), *v ? "[X]" : "[]");
			if (label_size.x > 0.0f) ImGui::RenderText(text_bb.GetTL(), label);

		}

		return pressed;
	}

	bool CheckboxSimplifiedEx(const char* label, bool* value)
	{
		assert(value);

		bool result = false;
		auto* drawList = ImGui::GetWindowDrawList();

		const auto pos = ImGui::GetCursorScreenPos();
		const auto mousePos = ImGui::GetMousePos();

		const auto itemSpacing = ImGui::GetStyle().ItemSpacing;
		const float lineHeight = ImGui::GetTextLineHeight();
		const float boxSize = std::floor(lineHeight * 1.3f);
		const float boxOffsetHorz = std::ceil(itemSpacing.x * 1.3f);
		const float boxOffsetVert = itemSpacing.y + std::floor(0.5f * (lineHeight - boxSize));
		const float clearance = boxSize * 0.2f;
		const auto corner = pos + ImVec2(boxOffsetHorz, boxOffsetVert);

		char buf[1024];
		strcpy(buf, label);
		for (int i = 0; i < sizeof(buf); ++i)
		{
			if (buf[i] == '#')
			{
				buf[i] = '\0';
				break;
			}
		}
		const float labelWidth = ImGui::CalcTextSize(buf).x;

		bool isHovered = ImRect(pos, pos + ImVec2(lineHeight + labelWidth + 2.0f * itemSpacing.x, lineHeight)).Contains(mousePos);

		ImVec4 color = ImGui::GetStyleColorVec4(ImGuiCol_Text);
		ImVec4 colorMark = color;
		color.w *= isHovered ? 1.0f : 0.25f;
		drawList->AddRect(corner, corner + ImVec2(boxSize, boxSize), ImColor(color), 0.0f, 0, 1.0f);

		if (isHovered && ImGui::IsWindowHovered())
		{
			if (ImGui::IsMouseClicked(0))
			{
				*value = !*value;
				result = true;
			}
		}

		if (*value)
		{
			ImVec2 checkMarkPts[3] = {
				corner + ImVec2(clearance, clearance + boxSize * 0.3f),
				corner + ImVec2(boxSize * 0.5f, boxSize - clearance),
				corner + ImVec2(boxSize - clearance, clearance),
			};
			drawList->AddPolyline(checkMarkPts, 3, ImColor(colorMark), false, 2.5f);
		}

		ImGui::Dummy(ImVec2(lineHeight + itemSpacing.x, lineHeight));

		if (strlen(buf) > 0)
		{
			ImGui::SameLine();
			ImGui::AlignTextToFramePadding();
			ImGui::Text(" %s", buf);
		}

		ImGui::SetCursorScreenPos(ImVec2(ImGui::GetCursorScreenPos().x, pos.y + ImGui::GetTextLineHeightWithSpacing() + itemSpacing.y));

		return result;
	}

	bool CheckboxSliderEx(const char* label, bool* v)
	{
		ImGui::Spacing();

		ImGuiWindow* window = ImGui::GetCurrentWindow();

		if (window->SkipItems) return false;

		ImGuiContext& g = *GImGui;
		const ImGuiStyle& style = ImGuiStyle::ImGuiStyle();
		const ImGuiID id = window->GetID(label);
		const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);
		const ImVec2 pading = ImVec2(2, 2);
		const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(label_size.y + style.FramePadding.x * 6, label_size.y + style.FramePadding.y / 2));

		ImGui::ItemSize(check_bb, style.FramePadding.y);

		ImRect total_bb = check_bb;
		if (label_size.x > 0) ImGui::SameLine(0, style.ItemInnerSpacing.x);

		const ImRect text_bb(window->DC.CursorPos + ImVec2(0, style.FramePadding.y), window->DC.CursorPos + ImVec2(0, style.FramePadding.y) + label_size);

		if (label_size.x > 0)
		{
			ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
			total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
		}

		if (!ImGui::ItemAdd(total_bb, id)) return false;

		bool hovered, held;
		bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);

		if (pressed) *v = !(*v);

		const ImVec4 enabled = ImVec4(1.00f, 1.00f, 1.00f, 1.0f);
		const ImVec4 disabled = ImVec4(0.2f, 0.2f, 0.2f, 1.0f);
		const ImVec4 enabledBg = ImVec4(0.70f, 0.70f, 0.70f, 1.0f);

		const float check_sz = ImMin(check_bb.GetWidth(), check_bb.GetHeight());
		const float check_sz2 = check_sz / 2;
		const float pad = ImMax(1.0f, (float)(int)(check_sz / 4.f));
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);
		window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(disabled), 12);

		if (*v)
		{
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 2, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 1, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 3, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 4, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 5, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabledBg), 12);
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 + 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabled), 12);
		}

		else
		{
			window->DrawList->AddCircleFilled(ImVec2(check_bb.Min.x + (check_bb.Max.x - check_bb.Min.x) / 2 - 6, check_bb.Min.y + 9), 7, ImGui::GetColorU32(enabled), 12);
		}

		if (label_size.x > 0.0f) ImGui::RenderText(text_bb.GetTL(), label);

		ImGui::Spacing();

		return pressed;
	}

	bool Vector3Control(const char* label, glm::vec3& values)
	{
		ImGui::PushID(label);

		constexpr ImVec4 colorX = ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f };
		constexpr ImVec4 colorY = ImVec4{ 0.25f, 0.7f, 0.2f, 1.0f };
		constexpr ImVec4 colorZ = ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f };

		// x
		{
			
			ImGui::PushStyleColor(ImGuiCol_Button, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorX);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorX);

			ImGui::SmallButton("X");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}
		
		// y
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorY);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorY);

			ImGui::SmallButton("Y");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}
		
		// z
		{
			ImGui::PushStyleColor(ImGuiCol_Button, colorZ);
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, colorZ);
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, colorZ);

			ImGui::SmallButton("Z");
			ImGui::SameLine();
			ImGui::PushItemWidth(50);
			ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
			ImGui::SameLine();
			ImGui::PopItemWidth();

			ImGui::PopStyleColor(3);
		}

		ImGui::NewLine();

		ImGui::PopID();

		return true;
	}
}