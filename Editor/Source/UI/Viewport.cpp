#include "Viewport.h"

#include "SceneHierarchy.h"
#include "TextureBrowser.h"

#include <array>

namespace Cosmos
{
	Viewport::Viewport(std::shared_ptr<Renderer> renderer, SceneHierarchy* sceneHierarcy, TextureBrowser* textureBrowser)
		: Widget("UI:Viewport"), mRenderer(renderer), mSceneHierarcy(sceneHierarcy), mTextureBrowser(textureBrowser)
	{
		VKCommander::GetInstance()->Insert("Viewport", std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());
		VKCommander::GetInstance()->SetMain("Viewport"); // set viewport renderpass to main renderpass
		VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->msaa = VK_SAMPLE_COUNT_1_BIT;
		
		LOG_TO_TERMINAL(Logger::Severity::Warn, "Rework Commander class and main RenderPass usage");

		mSurfaceFormat = VK_FORMAT_R8G8B8A8_SRGB;
		mDepthFormat = FindDepthFormat(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice());

		// render pass
		{
			std::array<VkAttachmentDescription, 2> attachments = {};

			// color attachment
			attachments[0].format = mSurfaceFormat;
			attachments[0].samples = VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->msaa;
			attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			// depth attachment
			attachments[1].format = mDepthFormat;
			attachments[1].samples = VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->msaa;
			attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
			attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
			attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkAttachmentReference colorReference = {};
			colorReference.attachment = 0;
			colorReference.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

			VkAttachmentReference depthReference = {};
			depthReference.attachment = 1;
			depthReference.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

			VkSubpassDescription subpassDescription = {};
			subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
			subpassDescription.colorAttachmentCount = 1;
			subpassDescription.pColorAttachments = &colorReference;
			subpassDescription.pDepthStencilAttachment = &depthReference;
			subpassDescription.inputAttachmentCount = 0;
			subpassDescription.pInputAttachments = nullptr;
			subpassDescription.preserveAttachmentCount = 0;
			subpassDescription.pPreserveAttachments = nullptr;
			subpassDescription.pResolveAttachments = nullptr;

			// Subpass dependencies for layout transitions
			std::array<VkSubpassDependency, 2> dependencies = {};

			dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[0].dstSubpass = 0;
			dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			dependencies[1].srcSubpass = 0;
			dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
			dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
			dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
			dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
			dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
			dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

			VkRenderPassCreateInfo renderPassCI = {};
			renderPassCI.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
			renderPassCI.attachmentCount = (uint32_t)attachments.size();
			renderPassCI.pAttachments = attachments.data();
			renderPassCI.subpassCount = 1;
			renderPassCI.pSubpasses = &subpassDescription;
			renderPassCI.dependencyCount = (uint32_t)dependencies.size();
			renderPassCI.pDependencies = dependencies.data();
			VK_ASSERT(vkCreateRenderPass(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &renderPassCI, nullptr, &VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->renderPass), "Failed to create renderpass");
		}

		// command pool
		{
			VKDevice::QueueFamilyIndices indices = std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->FindQueueFamilies
			(
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetPhysicalDevice(), 
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetSurface()
			);

			VkCommandPoolCreateInfo cmdPoolInfo = {};
			cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			cmdPoolInfo.queueFamilyIndex = indices.graphics.value();
			cmdPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			VK_ASSERT(vkCreateCommandPool(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &cmdPoolInfo, nullptr, &VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandPool), "Failed to create command pool");
		}

		// command buffers
		{
			VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandBuffers.resize(RENDERER_MAX_FRAMES_IN_FLIGHT);

			VkCommandBufferAllocateInfo cmdBufferAllocInfo = {};
			cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
			cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			cmdBufferAllocInfo.commandPool = VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandPool;
			cmdBufferAllocInfo.commandBufferCount = (uint32_t)VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandBuffers.size();
			VK_ASSERT(vkAllocateCommandBuffers(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &cmdBufferAllocInfo, VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandBuffers.data()), "Failed to allocate command buffers");
		}

		// sampler
		{
			VkSamplerCreateInfo samplerCI = {};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.magFilter = VK_FILTER_LINEAR;
			samplerCI.minFilter = VK_FILTER_LINEAR;
			samplerCI.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerCI.anisotropyEnable = VK_TRUE;
			samplerCI.maxAnisotropy = std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetProperties().limits.maxSamplerAnisotropy;
			samplerCI.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerCI.unnormalizedCoordinates = VK_FALSE;
			samplerCI.compareEnable = VK_FALSE;
			samplerCI.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			VK_ASSERT(vkCreateSampler(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &samplerCI, nullptr, &mSampler), "Failed to create sampler");
		}

		CreateResources();

		// must recreate global states to new primary commander specification
		mRenderer->CreateGlobalStates(); 
	}

	Viewport::~Viewport()
	{
		vkDeviceWaitIdle(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice());
		
		vkDestroySampler(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mSampler, nullptr);
		
		vkDestroyImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthView, nullptr);
		vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthMemory, nullptr);
		vkDestroyImage(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthImage, nullptr);
		
		for (size_t i = 0; i < std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetImages().size(); i++)
		{
			vkDestroyImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImageViews[i], nullptr);
			vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImageMemories[i], nullptr);
			vkDestroyImage(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImages[i], nullptr);
		}
	}

	void Viewport::OnUpdate()
	{
		if (ImGui::Begin("Viewport"))
		{
			DrawMenubar();

			ImGui::Image(mDescriptorSets[mRenderer->CurrentFrame()], ImGui::GetContentRegionAvail());
			
			// updating aspect ratio for the docking
			mCurrentSize = ImGui::GetWindowSize();
			Application::GetInstance()->GetCamera()->SetAspectRatio((float)(mCurrentSize.x / mCurrentSize.y));
			
			// viewport boundaries
			mContentRegionMin = ImGui::GetWindowContentRegionMin();
			mContentRegionMax = ImGui::GetWindowContentRegionMax();
			mContentRegionMin.x += ImGui::GetWindowPos().x;
			mContentRegionMin.y += ImGui::GetWindowPos().y;
			mContentRegionMax.x += ImGui::GetWindowPos().x;
			mContentRegionMax.y += ImGui::GetWindowPos().y;

			// draw gizmos on selected entity
			DrawGizmos();
		}

		ImGui::End();
	}

	void Viewport::OnEvent(Shared<Event> event)
	{
		if (event->GetType() == EventType::KeyboardPress)
		{
			auto camera = Application::GetInstance()->GetCamera();
			auto castedEvent = std::dynamic_pointer_cast<KeyboardPressEvent>(event);
			Keycode key = castedEvent->GetKeycode();

			// toggle editor viewport camera, move to viewport
			if (key == KEY_Z)
			{
				if (camera->CanMove() && camera->GetType() == Camera::Type::EDITOR_FLY)
				{
					camera->SetMove(false);
					Application::GetInstance()->GetWindow()->ToggleCursor(false);
					UIToggleCursor(false);
				}

				else if (!camera->CanMove() && camera->GetType() == Camera::Type::EDITOR_FLY)
				{
					camera->SetMove(true);
					Application::GetInstance()->GetWindow()->ToggleCursor(true);
					UIToggleCursor(true);
				}
			}
		}

		if (event->GetType() == EventType::WindowResize)
		{
			vkDestroyImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthView, nullptr);
			vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthMemory, nullptr);
			vkDestroyImage(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mDepthImage, nullptr);

			for (size_t i = 0; i < std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetImages().size(); i++)
			{
				vkDestroyImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImageViews[i], nullptr);
				vkFreeMemory(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImageMemories[i], nullptr);
				vkDestroyImage(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), mImages[i], nullptr);
			}

			for (auto& framebuffer : VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->frameBuffers)
			{
				vkDestroyFramebuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), framebuffer, nullptr);
			}

			CreateResources();
		}
	}

	void Viewport::CreateResources()
	{
		size_t size = std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetImages().size();

		// depth buffer
		{
			CreateImage
			(
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().width,
				std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().height,
				1,
				VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->msaa,
				mDepthFormat,
				VK_IMAGE_TILING_OPTIMAL,
				VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				mDepthImage,
				mDepthMemory
			);

			mDepthView = CreateImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), mDepthImage, mDepthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
		}

		// images
		{
			mImages.resize(size);
			mImageMemories.resize(size);
			mImageViews.resize(size);
			mDescriptorSets.resize(size);
			VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->frameBuffers.resize(size);

			for (size_t i = 0; i < size; i++)
			{
				CreateImage
				(
					std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(),
					std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().width,
					std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().height,
					1,
					VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->msaa,
					mSurfaceFormat,
					VK_IMAGE_TILING_OPTIMAL,
					VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
					VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
					mImages[i],
					mImageMemories[i]
				);

				VkCommandBuffer command = BeginSingleTimeCommand(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandPool);

				InsertImageMemoryBarrier
				(
					command,
					mImages[i],
					VK_ACCESS_TRANSFER_READ_BIT,
					VK_ACCESS_MEMORY_READ_BIT,
					VK_IMAGE_LAYOUT_UNDEFINED, // must get from last render pass (undefined also works)
					VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, // must set for next render pass
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VkImageSubresourceRange{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 }
				);

				EndSingleTimeCommand(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->commandPool, command);

				mImageViews[i] = CreateImageView(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice(), mImages[i], mSurfaceFormat, VK_IMAGE_ASPECT_COLOR_BIT);
				mDescriptorSets[i] = AddTexture(mSampler, mImageViews[i], VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

				std::array<VkImageView, 2> attachments = { mImageViews[i], mDepthView };

				VkFramebufferCreateInfo framebufferCI = {};
				framebufferCI.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
				framebufferCI.renderPass = VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->renderPass;
				framebufferCI.attachmentCount = (uint32_t)attachments.size();
				framebufferCI.pAttachments = attachments.data();
				framebufferCI.width = std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().width;
				framebufferCI.height = std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetSwapchain()->GetExtent().height;
				framebufferCI.layers = 1;
				VK_ASSERT(vkCreateFramebuffer(std::dynamic_pointer_cast<VKRenderer>(mRenderer)->GetDevice()->GetDevice(), &framebufferCI, nullptr, &VKCommander::GetInstance()->GetEntriesRef()["Viewport"]->frameBuffers[i]), "Failed to create framebuffer");
			}
		}
	}

	void Viewport::DrawMenubar()
	{
		// position window to menubar 
		ImVec2 winPos = ImGui::GetWindowSize();
		constexpr float gizmoInnerSpace = 7.5f;

		ImGui::PushID("##ViewportMenubar");
		ImGui::BeginGroup();
		
		// grid options
		{
			if (ImGui::Button(ICON_LC_AXIS_3D))
			{
				mGizmoType = GizmoType::UNDEFINED;
			}

			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - gizmoInnerSpace);
		
			if (ImGui::Button(ICON_LC_MOVE_3D))
			{
				mGizmoType = GizmoType::TRANSLATE;
			}
			
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - gizmoInnerSpace);

			if (ImGui::Button(ICON_LC_ROTATE_3D))
			{
				mGizmoType = GizmoType::ROTATE;
			}
			
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() - gizmoInnerSpace);

			if (ImGui::Button(ICON_LC_SCALE_3D))
			{
				mGizmoType = GizmoType::SCALE;
			}
		}
		
		ImGui::EndGroup();
		ImGui::PopID();
	}

	void Viewport::DrawGizmos()
	{
		// gizmos on entity
		Entity* selectedEntity = mSceneHierarcy->GetSelectedEntity();

		if (!selectedEntity || mGizmoType == GizmoType::UNDEFINED)
			return;

		if (!selectedEntity->HasComponent<TransformComponent>())
			return;

		ImGuizmo::SetOrthographic(false); // 3D engine dont have orthographic but perspective
		ImGuizmo::SetDrawlist();

		// viewport rect
		float windowWidth = (float)ImGui::GetWindowWidth();
		float windowHeight = (float)ImGui::GetWindowHeight();
		ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
		
		// camera
		auto camera = Application::GetInstance()->GetCamera();
		glm::mat4 view = camera->GetViewRef();
		glm::mat4 proj = glm::perspectiveRH(glm::radians(camera->GetFov()), mCurrentSize.x / mCurrentSize.y, camera->GetNear(), camera->GetFar());

		// entity
		auto& tc = selectedEntity->GetComponent<TransformComponent>();
		glm::mat4 transform = tc.GetTransform();

		// snapping
		bool snap = ImGui::IsKeyDown(ImGuiKey_LeftCtrl);
		float snapValue = mGizmoType == GizmoType::ROTATE ? 45.0f : 0.5f;
		float snapValues[3] = { snapValue, snapValue,snapValue };

		// gizmos drawing
		ImGuizmo::Manipulate
		(
			glm::value_ptr(view),
			glm::value_ptr(proj),
			(ImGuizmo::OPERATION)mGizmoType,
			ImGuizmo::MODE::LOCAL,
			glm::value_ptr(transform),
			nullptr,
			snap ? snapValues : nullptr
		);
		
		if (ImGuizmo::IsUsing())
		{
			glm::vec3 translation, rotation, scale;
			Decompose(transform, translation, rotation, scale);
		
			glm::vec3 deltaRotation = rotation - tc.rotation;
			tc.translation = translation;
			tc.rotation += deltaRotation;
			tc.scale = scale;
		}
	}
}