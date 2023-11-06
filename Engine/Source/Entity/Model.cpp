#include "Model.h"

#include "Core/Scene.h"
#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Texture::Texture(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{

	}

	Texture::~Texture()
	{
		Cleanup();
	}

	void Texture::UpdateDescriptor()
	{
		mDescriptor.imageLayout = mImageLayout;
		mDescriptor.imageView = mImageView;
		mDescriptor.sampler = mSampler;
	}

	void Texture::Cleanup()
	{
		vkDeviceWaitIdle(mRenderer->BackendDevice()->Device());

		vkDestroyImageView(mRenderer->BackendDevice()->Device(), mImageView, nullptr);
		vkDestroyImage(mRenderer->BackendDevice()->Device(), mImage, nullptr);
		vkFreeMemory(mRenderer->BackendDevice()->Device(), mImageMemory, nullptr);
		vkDestroySampler(mRenderer->BackendDevice()->Device(), mSampler, nullptr);
	}

	void Texture::LoadFromGLTFImage(tinygltf::Image& img, Sampler sampler)
	{
		unsigned char* buffer = nullptr;
		VkDeviceSize bufferSize = 0;
		bool deleteBuffer = false;

		if (img.component == 3)
		{
			// forcing alpha component
			LOG_TO_TERMINAL(Logger::Severity::Warn, "Only convert if device don't support RGB only");

			bufferSize = (VkDeviceSize)img.width * img.height * 4;
			buffer = new unsigned char[bufferSize];

			unsigned char* rgba = buffer;
			unsigned char* rgb = &img.image[0];

			// convert rgb to rgba
			for (int32_t i = 0; i < img.width * img.height; i++)
			{
				for (int32_t j = 0; j < 3; j++)
				{
					rgba[j] = rgb[j];
				}

				rgba += 4;
				rgb += 3;
			}

			deleteBuffer = true;
		}

		else
		{
			buffer = &img.image[0];
			bufferSize = img.image.size();
		}

		mWidth = img.width;
		mHeight = img.height;
		mMipLevels = static_cast<uint32_t>(floor(log2(std::max(mWidth, mHeight))) + 1.0);

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;
		VkFormatProperties properties = {};
		vkGetPhysicalDeviceFormatProperties(mRenderer->BackendDevice()->PhysicalDevice(), format, &properties);
		LOG_ASSERT(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_SRC_BIT, "Physical device doesnt support blit from source");
		LOG_ASSERT(properties.optimalTilingFeatures & VK_FORMAT_FEATURE_BLIT_DST_BIT, "Physical device doesnt support blit from destiny");

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Move all devices requirements to device creation");

		VkBuffer stagingBuffer;
		VkDeviceMemory stagingMemory;
		uint8_t* data = nullptr;

		// create staging buffers
		{
			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.pNext = nullptr;
			bufferCI.flags = 0;
			bufferCI.size = bufferSize;
			bufferCI.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VK_ASSERT(vkCreateBuffer(mRenderer->BackendDevice()->Device(), &bufferCI, nullptr, &stagingBuffer), "Failed to create buffer");

			VkMemoryRequirements memReqs = {};
			vkGetBufferMemoryRequirements(mRenderer->BackendDevice()->Device(), stagingBuffer, &memReqs);

			VkMemoryAllocateInfo memAllocInfo = {};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAllocInfo.pNext = nullptr;
			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = mRenderer->BackendDevice()->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
			VK_ASSERT(vkAllocateMemory(mRenderer->BackendDevice()->Device(), &memAllocInfo, nullptr, &stagingMemory), "Failed to allocate memory for staging buffer");
			VK_ASSERT(vkBindBufferMemory(mRenderer->BackendDevice()->Device(), stagingBuffer, stagingMemory, 0), "Failed to bind staging buffer with staging memory");

			// copy staging buffer to mapped memory and unmaps it
			VK_ASSERT(vkMapMemory(mRenderer->BackendDevice()->Device(), stagingMemory, 0, memReqs.size, 0, (void**)&data), "Failed to map staging memory for copy");
			memcpy(data, buffer, bufferSize);
			vkUnmapMemory(mRenderer->BackendDevice()->Device(), stagingMemory);
		}

		LOG_TO_TERMINAL(Logger::Severity::Warn, "Check for MSAA when creating image");

		// create image
		{
			VkImageCreateInfo imageCI = {};
			imageCI.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageCI.pNext = nullptr;
			imageCI.flags = 0;
			imageCI.format = format;
			imageCI.mipLevels = mMipLevels;
			imageCI.arrayLayers = 1;
			imageCI.samples = VK_SAMPLE_COUNT_1_BIT; // check for MSAA ?
			imageCI.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageCI.usage = VK_IMAGE_USAGE_SAMPLED_BIT;
			imageCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageCI.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageCI.extent.width = mWidth;
			imageCI.extent.height = mHeight;
			imageCI.extent.depth = 1;
			imageCI.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			VK_ASSERT(vkCreateImage(mRenderer->BackendDevice()->Device(), &imageCI, nullptr, &mImage), "Failed to create image");

			VkMemoryRequirements memReqs = {};
			vkGetBufferMemoryRequirements(mRenderer->BackendDevice()->Device(), stagingBuffer, &memReqs);

			VkMemoryAllocateInfo memAllocInfo = {};
			memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memAllocInfo.pNext = nullptr;
			memAllocInfo.allocationSize = memReqs.size;
			memAllocInfo.memoryTypeIndex = mRenderer->BackendDevice()->GetMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
			VK_ASSERT(vkAllocateMemory(mRenderer->BackendDevice()->Device(), &memAllocInfo, nullptr, &mImageMemory), "Failed to allocate memory for image");
			VK_ASSERT(vkBindImageMemory(mRenderer->BackendDevice()->Device(), mImage, mImageMemory, 0), "Failed to bind memory with image");
		}

		// record the command buffer
		VkCommandBuffer cmd = CreateCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
		);

		VkImageSubresourceRange subresourceRange = {};
		subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		subresourceRange.levelCount = 1;
		subresourceRange.layerCount = 1;

		// put image barrier for layout transition and copy
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.srcAccessMask = 0;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.image = mImage;
			barrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		// copy the staging buffer to the image 
		VkBufferImageCopy bufferCopyRegion = {};
		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferCopyRegion.imageSubresource.mipLevel = 0;
		bufferCopyRegion.imageSubresource.baseArrayLayer = 0;
		bufferCopyRegion.imageSubresource.layerCount = 1;
		bufferCopyRegion.imageExtent.width = mWidth;
		bufferCopyRegion.imageExtent.height = mHeight;
		bufferCopyRegion.imageExtent.depth = 1;
		vkCmdCopyBufferToImage(cmd, stagingBuffer, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferCopyRegion);

		// put image barrier for layout transition and paste
		{
			VkImageMemoryBarrier barrier{};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.image = mImage;
			barrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		// send the command buffer to be processed
		FlushCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			cmd,
			mRenderer->BackendDevice()->GraphicsQueue(),
			true
		);

		// generate the mipmap chain
		VkCommandBuffer blitCmd = CreateCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
		);

		for (uint32_t i = 0; i < mMipLevels; i++)
		{
			VkImageBlit imageBlit = {};

			imageBlit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.srcSubresource.layerCount = 1;
			imageBlit.srcSubresource.mipLevel = i - 1;
			imageBlit.srcOffsets[1].x = int32_t(mWidth >> (i - 1));
			imageBlit.srcOffsets[1].y = int32_t(mHeight >> (i - 1));
			imageBlit.srcOffsets[1].z = 1;

			imageBlit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageBlit.dstSubresource.layerCount = 1;
			imageBlit.dstSubresource.mipLevel = i;
			imageBlit.dstOffsets[1].x = int32_t(mWidth >> i);
			imageBlit.dstOffsets[1].y = int32_t(mHeight >> i);
			imageBlit.dstOffsets[1].z = 1;

			VkImageSubresourceRange mipSubRange = {};
			mipSubRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			mipSubRange.baseMipLevel = i;
			mipSubRange.levelCount = 1;
			mipSubRange.layerCount = 1;

			// insert memory barrier to transfer dst
			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.srcAccessMask = 0;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.image = mImage;
				barrier.subresourceRange = mipSubRange;
				vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}

			// perform the blit
			vkCmdBlitImage(blitCmd, mImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageBlit, VK_FILTER_LINEAR);

			// insert memory barrier to set the image to read only
			{
				VkImageMemoryBarrier barrier = {};
				barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
				barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
				barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
				barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
				barrier.image = mImage;
				barrier.subresourceRange = mipSubRange;
				vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
			}
		}

		subresourceRange.levelCount = mMipLevels;
		mImageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		// insert memory barrier to read only optimal
		{
			VkImageMemoryBarrier barrier = {};
			barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.image = mImage;
			barrier.subresourceRange = subresourceRange;
			vkCmdPipelineBarrier(blitCmd, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier);
		}

		FlushCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			blitCmd,
			mRenderer->BackendDevice()->GraphicsQueue(),
			true
		);

		// create sampler
		{
			VkSamplerCreateInfo samplerCI = {};
			samplerCI.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerCI.magFilter = sampler.magFilter;
			samplerCI.minFilter = sampler.minFilter;
			samplerCI.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerCI.addressModeU = sampler.addressModeU;
			samplerCI.addressModeV = sampler.addressModeV;
			samplerCI.addressModeW = sampler.addressModeW;
			samplerCI.compareOp = VK_COMPARE_OP_NEVER;
			samplerCI.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			samplerCI.maxAnisotropy = 1.0;
			samplerCI.anisotropyEnable = VK_FALSE;
			samplerCI.maxLod = (float)mMipLevels;
			samplerCI.maxAnisotropy = 8.0f;
			samplerCI.anisotropyEnable = VK_TRUE;
			VK_ASSERT(vkCreateSampler(mRenderer->BackendDevice()->Device(), &samplerCI, nullptr, &mSampler), "Failed to create sampler");
		}

		LOG_TO_TERMINAL(Logger::Severity::Warn, "TODO: Modify CreateImageView() to support custom miplevels");

		// create image view
		{
			VkImageViewCreateInfo imageViewCI = {};
			imageViewCI.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
			imageViewCI.image = mImage;
			imageViewCI.viewType = VK_IMAGE_VIEW_TYPE_2D;
			imageViewCI.format = format;
			imageViewCI.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_G, VK_COMPONENT_SWIZZLE_B, VK_COMPONENT_SWIZZLE_A };
			imageViewCI.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageViewCI.subresourceRange.layerCount = 1;
			imageViewCI.subresourceRange.levelCount = mMipLevels;
			VK_ASSERT(vkCreateImageView(mRenderer->BackendDevice()->Device(), &imageViewCI, nullptr, &mImageView), "Failed to create image view");
		}

		// update descriptor
		{
			mDescriptor.sampler = mSampler;
			mDescriptor.imageView = mImageView;
			mDescriptor.imageLayout = mImageLayout;
		}

		vkFreeMemory(mRenderer->BackendDevice()->Device(), stagingMemory, nullptr);
		vkDestroyBuffer(mRenderer->BackendDevice()->Device(), stagingBuffer, nullptr);

		if (deleteBuffer)
		{
			delete[] buffer;
		}
	}

	Primitive::Primitive(Material& material, uint32_t firstIndex, uint32_t indexCount, uint32_t vertexCount)
		: mMaterial(material), mFirstIndex(firstIndex), mIndexCount(indexCount), mVertexCount(vertexCount)
	{
		mHasIndices = indexCount > 0 ? true : false;
	}

	void Primitive::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		mBB.SetMin(min);
		mBB.SetMax(max);
		mBB.SetValid(true);
	}

	Mesh::Mesh(std::shared_ptr<Renderer>& renderer, glm::mat4 matrix)
		: mRenderer(renderer)
	{
		mUniformBlock.matrix = matrix;

		VK_ASSERT(BufferCreate(mRenderer->BackendDevice(),
			VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			sizeof(UniformBlock),
			&mUBO.buffer,
			&mUBO.memory,
			&mUniformBlock
		), "Failed to create buffer");
	}

	Mesh::~Mesh()
	{
		vkDestroyBuffer(mRenderer->BackendDevice()->Device(), mUBO.buffer, nullptr);
		vkFreeMemory(mRenderer->BackendDevice()->Device(), mUBO.memory, nullptr);

		for (Primitive* p : mPrimitives)
		{
			delete p;
		}
	}

	void Mesh::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		mBB.SetMin(min);
		mBB.SetMax(max);
		mBB.SetValid(true);
	}

	GLTFNode::~GLTFNode()
	{
		if (mMesh != nullptr)
		{
			delete mMesh;
		}

		for (auto& child : mChildren)
		{
			delete child;
		}
	}

	void GLTFNode::Update()
	{
		if (mMesh != nullptr)
		{
			glm::mat4 m = GetMatrix();

			if (mSkin != nullptr)
			{
				mMesh->GetUniformBlock();

				// update join matrices
				glm::mat4 inverseTransform = glm::inverse(m);
				size_t numJoints = std::min((uint32_t)mSkin->JointNodes().size(), MAX_NUM_JOINTS);

				for (size_t i = 0; i < numJoints; i++)
				{
					GLTFNode* jointNode = mSkin->JointNodes()[i];
					glm::mat4 jointMat = jointNode->GetMatrix() * mSkin->InverseBindMatrices()[i];
					jointMat = inverseTransform * jointMat;
					mMesh->GetUniformBlock().jointMatrix[i] = jointMat;
				}

				mMesh->GetUniformBlock().jointCount = (float)numJoints;
				memcpy(mMesh->GetUBO().mapped, &mMesh->GetUniformBlock(), sizeof(mMesh->GetUniformBlock()));
			}

			else
			{
				memcpy(mMesh->GetUBO().mapped, &m, sizeof(glm::mat4));
			}
		}

		for (auto& child : mChildren)
		{
			child->Update();
		}
	}

	glm::mat4 GLTFNode::LocalMatrix()
	{
		return glm::translate(glm::mat4(1.0f), mTranslation) * glm::mat4(mRotation) * glm::scale(glm::mat4(1.0f), mScale) * mMatrix;
	}

	glm::mat4 GLTFNode::GetMatrix()
	{
		glm::mat4 m = LocalMatrix();

		GLTFNode* p = mParent;

		while (p != nullptr)
		{
			m = p->LocalMatrix() * m;
			p = p->mParent;
		}

		return m;
	}

	Model::Model(std::shared_ptr<Renderer>& renderer, Scene* scene)
		: Entity(mScene), mRenderer(renderer), mScene(scene)
	{
		Logger() << "Creating Model";
	}

	Model::~Model()
	{
		Destroy();
	}

	void Model::OnRenderDraw()
	{
	}

	void Model::OnUpdate(float timestep)
	{
	}

	void Model::OnDestroy()
	{
	}

	void Model::DrawNode(GLTFNode* node, VkCommandBuffer commandBuffer)
	{
		if (node->GetMesh() != nullptr)
		{
			for (Primitive* primitive : node->GetMesh()->Primitives())
			{
				vkCmdDrawIndexed(commandBuffer, primitive->IndexCount(), 1, primitive->FirstIndex(), 0, 0);
			}
		}

		for (auto& child : node->Children())
		{
			DrawNode(child, commandBuffer);
		}
	}

	void Model::Draw(VkCommandBuffer commandBuffer)
	{
		const VkDeviceSize offsets[1] = { 0 };

		vkCmdBindVertexBuffers(commandBuffer, 0, 1, &mVertices.buffer, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mIndices.buffer, 0, VK_INDEX_TYPE_UINT32);

		for (auto& node : mNodes)
		{
			DrawNode(node, commandBuffer);
		}
	}

	void Model::UpdateAnimations(uint32_t index, float time)
	{
		if (mAnimations.empty())
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "This model does not have animations");
			return;
		}

		if (index > (uint32_t)mAnimations.size() - 1)
		{
			LOG_TO_TERMINAL(Logger::Severity::Warn, "There's no animation with index %d", index);
			return;
		}

		Animation& animation = mAnimations[index];
		bool updated = false;

		for (auto& channel : animation.Channels())
		{
			Animation::Sampler& sampler = animation.Samplers()[channel.samplerIndex];

			if (sampler.inputs.size() > sampler.outputs.size())
			{
				continue;
			}

			for (size_t i = 0; i < sampler.inputs.size() - 1; i++)
			{
				if ((time >= sampler.inputs[i]) && (time <= sampler.inputs[i + 1]))
				{
					float u = std::max(0.0f, time - sampler.inputs[i]) / (sampler.inputs[i + 1] - sampler.inputs[i]);

					if (u <= 1.0f)
					{
						switch (channel.pathType)
						{

						case Animation::Channel::PathType::PATH_TYPE_TRANSLATION:
						{
							glm::vec4 trans = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], u);
							channel.node->Translation() = glm::vec3(trans);
							break;
						}

						case Animation::Channel::PathType::PATH_TYPE_SCALE:
						{
							glm::vec4 trans = glm::mix(sampler.outputs[i], sampler.outputs[i + 1], u);
							channel.node->Scale() = glm::vec3(trans);
							break;
						}

						case Animation::Channel::PathType::PATH_TYPE_ROTATION:
						{
							glm::quat q1;							q1.x = sampler.outputs[i].x;
							q1.y = sampler.outputs[i].y;
							q1.z = sampler.outputs[i].z;
							q1.w = sampler.outputs[i].w;

							glm::quat q2;
							q2.x = sampler.outputs[i + 1].x;
							q2.y = sampler.outputs[i + 1].y;
							q2.z = sampler.outputs[i + 1].z;
							q2.w = sampler.outputs[i + 1].w;

							channel.node->Rotation() = glm::normalize(glm::slerp(q1, q2, u));
							break;
						}

						}
						updated = true;
					}
				}
			}
		}

		if (updated)
		{
			for (auto& node : mNodes)
			{
				node->Update();
			}
		}
	}

	void Model::LoadFromFile(std::string path, float scale)
	{
		tinygltf::Model mdl;
		tinygltf::TinyGLTF gltfContext;

		std::string error;
		std::string warning;

		bool binary = false;
		size_t extpos = path.rfind('.', path.length());

		if (extpos != std::string::npos)
		{
			binary = (path.substr(extpos + 1, path.length() - extpos) == "glb");
		}

		bool fileLoaded = binary ? gltfContext.LoadBinaryFromFile(&mdl, &error, &warning, path.c_str()) : gltfContext.LoadASCIIFromFile(&mdl, &error, &warning, path.c_str());

		LoaderInfo loaderInfo = {};
		size_t vertexCount = 0;
		size_t indexCount = 0;

		if (fileLoaded)
		{
			LoadTextureSamplers(mdl);
			LoadTextures(mdl);
			LoadMaterials(mdl);

			const tinygltf::Scene& scene = mdl.scenes[mdl.defaultScene > -1 ? mdl.defaultScene : 0];

			// get vertex and index buffer sizes up-front

			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				NodeProperties(mdl.nodes[scene.nodes[i]], mdl, vertexCount, indexCount);
			}

			loaderInfo.vertexBuffer = new Vertex[vertexCount];
			loaderInfo.indexBuffer = new uint32_t[indexCount];

			// TODO: scene handling with no default scene
			for (size_t i = 0; i < scene.nodes.size(); i++)
			{
				const tinygltf::Node node = mdl.nodes[scene.nodes[i]];
				LoadNode(nullptr, node, scene.nodes[i], mdl, loaderInfo, scale);
			}

			if (mdl.animations.size() > 0)
			{
				LoadAnimations(mdl);
			}

			LoadSkins(mdl);

			for (auto node : mLinearNodes)

			{
				// assign skins
				if (node->SkinIndex() > -1)
				{
					node->GetSkin() = mSkins[node->SkinIndex()];
				}

				// initial pose
				if (node->GetMesh())
				{
					node->Update();
				}
			}
		}

		else
		{
			LOG_TO_TERMINAL(Logger::Severity::Error, "Could not load model");
			return;
		}

		mExtensions = mdl.extensionsUsed;

		size_t vertexBufferSize = vertexCount * sizeof(Vertex);
		size_t indexBufferSize = indexCount * sizeof(uint32_t);

		LOG_ASSERT(vertexBufferSize > 0, "Vertex buffer is zero, something went wrong");

		struct StagingBuffer
		{
			VkBuffer buffer;
			VkDeviceMemory memory;
		} vertexStaging = {}, indexStaging = {};

		// staging vertex buffer
		VK_ASSERT
		(
			BufferCreate
			(
				mRenderer->BackendDevice(),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				(VkDeviceSize)vertexBufferSize,
				&vertexStaging.buffer,
				&vertexStaging.memory,
				loaderInfo.vertexBuffer
			), "Failed to create Staging Vertex Buffer"
		);

		// staging index buffer
		VK_ASSERT
		(
			BufferCreate
			(
				mRenderer->BackendDevice(),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				(VkDeviceSize)indexBufferSize,
				&indexStaging.buffer,
				&indexStaging.memory,
				loaderInfo.indexBuffer
			), "Failed to create Staging Index Buffer"
		);

		// gpu vertex buffer
		VK_ASSERT
		(
			BufferCreate
			(
				mRenderer->BackendDevice(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				(VkDeviceSize)vertexBufferSize,
				&mVertices.buffer,
				&mVertices.memory
			), "Failed to create GPU Vertex Buffer"
		);

		// gpu index buffer (if has any)
		if (indexBufferSize > 1)
		{
			VK_ASSERT
			(
				BufferCreate
				(
					mRenderer->BackendDevice(),
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					(VkDeviceSize)indexBufferSize,
					&mIndices.buffer,
					&mIndices.memory
				), "Failed to create GPU Index Buffer"
			);
		}

		// copy from staging buffers
		LOG_TO_TERMINAL(Logger::Severity::Warn, "Create custom command entry for models. using swapchain's (witch also should be modified)");
		VkCommandBuffer copyCmd = CreateCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			true
		);

		VkBufferCopy copyRegion = {};
		copyRegion.size = vertexBufferSize;
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, mVertices.buffer, 1, &copyRegion);

		if (indexBufferSize > 0)
		{
			copyRegion.dstOffset = 0;
			copyRegion.srcOffset = 0;
			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(copyCmd, indexStaging.buffer, mIndices.buffer, 1, &copyRegion);
		}

		FlushCommandBuffer
		(
			mRenderer->BackendDevice(),
			mRenderer->BackendDevice()->MainCommandEntry()->commandPool,
			copyCmd,
			mRenderer->BackendDevice()->GraphicsQueue(),
			true
		);

		// free staging buffers
		vkDestroyBuffer(mRenderer->BackendDevice()->Device(), vertexStaging.buffer, nullptr);
		vkFreeMemory(mRenderer->BackendDevice()->Device(), vertexStaging.memory, nullptr);;

		if (indexBufferSize > 0)
		{
			vkDestroyBuffer(mRenderer->BackendDevice()->Device(), indexStaging.buffer, nullptr);
			vkFreeMemory(mRenderer->BackendDevice()->Device(), indexStaging.memory, nullptr);
		}

		delete[] loaderInfo.vertexBuffer;
		delete[] loaderInfo.indexBuffer;

		GetModelDimensions();
	}

	void Model::Destroy()
	{
		if (mVertices.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(mRenderer->BackendDevice()->Device(), mVertices.buffer, nullptr);
			vkFreeMemory(mRenderer->BackendDevice()->Device(), mVertices.memory, nullptr);
			mVertices.buffer = VK_NULL_HANDLE;
			mVertices.memory = VK_NULL_HANDLE;
		}

		if (mIndices.buffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(mRenderer->BackendDevice()->Device(), mIndices.buffer, nullptr);
			vkFreeMemory(mRenderer->BackendDevice()->Device(), mIndices.memory, nullptr);
			mVertices.buffer = VK_NULL_HANDLE;
			mVertices.memory = VK_NULL_HANDLE;
		}

		for (auto& tex : mTextures)
		{
			tex.Cleanup();
		}

		mTextures.clear(); //mTextures.resize(0) breaks as there's no default constructor
		mTextureSamplers.resize(0);

		for (auto& node : mNodes)
		{
			delete node;
		}

		mMaterials.resize(0);
		mAnimations.resize(0);
		mLinearNodes.resize(0);
		mExtensions.resize(0);

		for (auto& skin : mSkins)
		{
			delete skin;
		}

		mSkins.resize(0);
	}

	void Model::LoadNode(GLTFNode* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& mdl, LoaderInfo& loaderInfo, float globalScale)
	{
		GLTFNode* newNode = new GLTFNode{};
		newNode->Index() = nodeIndex;
		newNode->GetParent() = parent;
		newNode->Name() = node.name;
		newNode->SkinIndex() = node.skin;
		newNode->Matrix() = glm::mat4(1.0f);

		// generate local node matrix

		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3)
		{
			translation = glm::make_vec3(node.translation.data());
			newNode->Translation() = translation;
		}

		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->Rotation() = glm::mat4(q);
		}

		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
			newNode->Scale() = scale;
		}

		if (node.matrix.size() == 16)
		{
			newNode->Matrix() = glm::make_mat4x4(node.matrix.data());
		};

		// node with children
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				LoadNode(newNode, mdl.nodes[node.children[i]], node.children[i], mdl, loaderInfo, globalScale);
			}
		}

		// node contains mesh data

		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = mdl.meshes[node.mesh];
			Mesh* newMesh = new Mesh(mRenderer, newNode->Matrix());

			for (size_t j = 0; j < mesh.primitives.size(); j++)
			{
				const tinygltf::Primitive& primitive = mesh.primitives[j];
				uint32_t vertexStart = (uint32_t)loaderInfo.vertexPos;
				uint32_t indexStart = (uint32_t)loaderInfo.indexPos;
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin = {};
				glm::vec3 posMax = {};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;

				// vertices
				{
					const float* bufferPos = nullptr;
					const float* bufferNormals = nullptr;
					const float* bufferTexCoordSet0 = nullptr;
					const float* bufferTexCoordSet1 = nullptr;
					const float* bufferColorSet0 = nullptr;
					const void* bufferJoints = nullptr;
					const float* bufferWeights = nullptr;

					int posByteStride;
					int normByteStride;
					int uv0ByteStride;
					int uv1ByteStride;
					int color0ByteStride;
					int jointByteStride;
					int weightByteStride;

					int jointComponentType;

					// position (required)
					LOG_ASSERT(primitive.attributes.find("POSITION") != primitive.attributes.end(), "Position is required for loading a model");

					const tinygltf::Accessor& posAccessor = mdl.accessors[primitive.attributes.find("POSITION")->second];
					const tinygltf::BufferView& posView = mdl.bufferViews[posAccessor.bufferView];

					bufferPos = reinterpret_cast<const float*>(&(mdl.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
					posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
					posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
					vertexCount = static_cast<uint32_t>(posAccessor.count);
					posByteStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

					if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
					{
						const tinygltf::Accessor& normAccessor = mdl.accessors[primitive.attributes.find("NORMAL")->second];
						const tinygltf::BufferView& normView = mdl.bufferViews[normAccessor.bufferView];
						bufferNormals = reinterpret_cast<const float*>(&(mdl.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
						normByteStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// UV0
					if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& uvAccessor = mdl.accessors[primitive.attributes.find("TEXCOORD_0")->second];
						const tinygltf::BufferView& uvView = mdl.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet0 = reinterpret_cast<const float*>(&(mdl.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv0ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// UV1
					if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
					{
						const tinygltf::Accessor& uvAccessor = mdl.accessors[primitive.attributes.find("TEXCOORD_1")->second];
						const tinygltf::BufferView& uvView = mdl.bufferViews[uvAccessor.bufferView];
						bufferTexCoordSet1 = reinterpret_cast<const float*>(&(mdl.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
						uv1ByteStride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
					}

					// vertex color
					if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& accessor = mdl.accessors[primitive.attributes.find("COLOR_0")->second];
						const tinygltf::BufferView& view = mdl.bufferViews[accessor.bufferView];
						bufferColorSet0 = reinterpret_cast<const float*>(&(mdl.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
						color0ByteStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
					}

					// joints
					if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& jointAccessor = mdl.accessors[primitive.attributes.find("JOINTS_0")->second];
						const tinygltf::BufferView& jointView = mdl.bufferViews[jointAccessor.bufferView];
						bufferJoints = &(mdl.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
						jointComponentType = jointAccessor.componentType;
						jointByteStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					// weights
					if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
					{
						const tinygltf::Accessor& weightAccessor = mdl.accessors[primitive.attributes.find("WEIGHTS_0")->second];
						const tinygltf::BufferView& weightView = mdl.bufferViews[weightAccessor.bufferView];
						bufferWeights = reinterpret_cast<const float*>(&(mdl.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
						weightByteStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
					}

					hasSkin = (bufferJoints && bufferWeights);

					for (size_t v = 0; v < posAccessor.count; v++)
					{
						Vertex& vert = loaderInfo.vertexBuffer[loaderInfo.vertexPos];
						vert.pos = glm::vec4(glm::make_vec3(&bufferPos[v * posByteStride]), 1.0f);

						vert.normal = glm::normalize(glm::vec3(bufferNormals ? glm::make_vec3(&bufferNormals[v * normByteStride]) : glm::vec3(0.0f)));
						vert.uv0 = bufferTexCoordSet0 ? glm::make_vec2(&bufferTexCoordSet0[v * uv0ByteStride]) : glm::vec3(0.0f);
						vert.uv1 = bufferTexCoordSet1 ? glm::make_vec2(&bufferTexCoordSet1[v * uv1ByteStride]) : glm::vec3(0.0f);
						vert.color = bufferColorSet0 ? glm::make_vec4(&bufferColorSet0[v * color0ByteStride]) : glm::vec4(1.0f);

						if (hasSkin)
						{
							switch (jointComponentType)
							{
							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
							{
								const uint16_t* buf = (const uint16_t*)bufferJoints;
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}

							case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
							{
								const uint8_t* buf = (const uint8_t*)bufferJoints;
								vert.joint0 = glm::vec4(glm::make_vec4(&buf[v * jointByteStride]));
								break;
							}

							default:

								// Not supported by spec
								LOG_TO_TERMINAL(Logger::Severity::Error, "Joint component type %d is not supported", jointComponentType);
								break;
							}
						}

						else
						{
							vert.joint0 = glm::vec4(0.0f);
						}

						vert.weight0 = hasSkin ? glm::make_vec4(&bufferWeights[v * weightByteStride]) : glm::vec4(0.0f);

						// fix for all zero weights
						if (glm::length(vert.weight0) == 0.0f)
						{
							vert.weight0 = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);
						}

						loaderInfo.vertexPos++;
					}
				}

				// indices
				if (hasIndices)
				{
					const tinygltf::Accessor& accessor = mdl.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];

					indexCount = (uint32_t)accessor.count;
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const uint32_t* buf = (const uint32_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}

						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t* buf = (const uint16_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}

						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t* buf = (const uint8_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							loaderInfo.indexBuffer[loaderInfo.indexPos] = buf[index] + vertexStart;
							loaderInfo.indexPos++;
						}

						break;
					}

					default:
						LOG_TO_TERMINAL(Logger::Severity::Error, "Index component type %d is not supported", accessor.componentType);
						return;
					}
				}

				Material& mat = primitive.material > -1 ? mMaterials[primitive.material] : mMaterials.back();
				Primitive* newPrimitive = new Primitive(mat, indexStart, indexCount, vertexCount);
				newPrimitive->SetBoundingBox(posMin, posMax);

				newMesh->Primitives().push_back(newPrimitive);
			}

			// mesh BB from BBs of primitives
			for (auto p : newMesh->Primitives())
			{
				if (p->BB().IsValid() && !newMesh->BB().IsValid())
				{
					newMesh->BB() = p->BB();
					newMesh->BB().SetValid(true);
				}

				newMesh->BB().SetMin(glm::min(newMesh->BB().GetMin(), p->BB().GetMin()));
				newMesh->BB().SetMax(glm::max(newMesh->BB().GetMax(), p->BB().GetMax()));
			}

			newNode->GetMesh() = newMesh;
		}

		if (parent)
		{
			parent->Children().push_back(newNode);
		}

		else
		{
			mNodes.push_back(newNode);
		}

		mLinearNodes.push_back(newNode);
	}

	void Model::LoadSkins(tinygltf::Model& mdl)
	{
		for (tinygltf::Skin& source : mdl.skins)
		{
			Skin* newSkin = new Skin();
			newSkin->Name() = source.name;

			// find skeleton root node
			if (source.skeleton > -1)
			{
				newSkin->Root() = NodeFromIndex(source.skeleton);
			}

			// find joint nodes
			for (int jointIndex : source.joints)
			{
				GLTFNode* node = NodeFromIndex(jointIndex);

				if (node != nullptr)
				{
					newSkin->JointNodes().push_back(NodeFromIndex(jointIndex));
				}
			}

			// get inverse bind matrices from buffer

			if (source.inverseBindMatrices > -1)
			{
				const tinygltf::Accessor& accessor = mdl.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];
				newSkin->InverseBindMatrices().resize(accessor.count);
				memcpy(newSkin->InverseBindMatrices().data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			mSkins.push_back(newSkin);
		}
	}

	void Model::LoadTextures(tinygltf::Model& mdl)
	{
		for (tinygltf::Texture& tex : mdl.textures)
		{
			tinygltf::Image image = mdl.images[tex.source];
			Texture::Sampler textureSampler;

			if (tex.sampler == -1)
			{
				// no sampler specified, use a default one
				textureSampler.magFilter = VK_FILTER_LINEAR;
				textureSampler.minFilter = VK_FILTER_LINEAR;
				textureSampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
				textureSampler.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			}

			else
			{
				textureSampler = mTextureSamplers[tex.sampler];
			}

			Texture texture(mRenderer);
			texture.LoadFromGLTFImage(image, textureSampler);
			mTextures.push_back(texture);
		}
	}

	void Model::LoadTextureSamplers(tinygltf::Model& mdl)
	{
		for (tinygltf::Sampler smpl : mdl.samplers)
		{
			Texture::Sampler sampler = {};
			sampler.minFilter = FilterMode(smpl.minFilter);
			sampler.magFilter = FilterMode(smpl.magFilter);
			sampler.addressModeU = WrapMode(smpl.wrapS);
			sampler.addressModeV = WrapMode(smpl.wrapT);
			sampler.addressModeW = sampler.addressModeV;
			mTextureSamplers.push_back(sampler);
		}
	}

	void Model::LoadMaterials(tinygltf::Model& mdl)
	{
		for (tinygltf::Material& mat : mdl.materials)
		{
			Material material = {};
			material.DoubleSized() = mat.doubleSided;

			if (mat.values.find("baseColorTexture") != mat.values.end())
			{
				material.BaseTexture() = &mTextures[mat.values["baseColorTexture"].TextureIndex()];
				material.TexCoords().baseColor = mat.values["baseColorTexture"].TextureTexCoord();
			}

			if (mat.values.find("metallicRoughnessTexture") != mat.values.end())
			{
				material.MetallicRoughnessTexture() = &mTextures[mat.values["metallicRoughnessTexture"].TextureIndex()];
				material.TexCoords().metallicRoughness = mat.values["metallicRoughnessTexture"].TextureTexCoord();
			}

			if (mat.values.find("roughnessFactor") != mat.values.end())
			{
				material.RoughnessFactor() = (float)mat.values["roughnessFactor"].Factor();
			}

			if (mat.values.find("metallicFactor") != mat.values.end())
			{
				material.MetallicFactor() = (float)mat.values["metallicFactor"].Factor();
			}

			if (mat.values.find("baseColorFactor") != mat.values.end())
			{
				material.BaseColor() = glm::make_vec4(mat.values["baseColorFactor"].ColorFactor().data());
			}

			if (mat.additionalValues.find("normalTexture") != mat.additionalValues.end())
			{
				material.NormalTexture() = &mTextures[mat.additionalValues["normalTexture"].TextureIndex()];
				material.TexCoords().normal = mat.additionalValues["normalTexture"].TextureTexCoord();
			}

			if (mat.additionalValues.find("emissiveTexture") != mat.additionalValues.end())
			{
				material.EmissiveTexture() = &mTextures[mat.additionalValues["emissiveTexture"].TextureIndex()];
				material.TexCoords().emissive = mat.additionalValues["emissiveTexture"].TextureTexCoord();
			}

			if (mat.additionalValues.find("occlusionTexture") != mat.additionalValues.end())
			{
				material.OcclusionTexture() = &mTextures[mat.additionalValues["occlusionTexture"].TextureIndex()];
				material.TexCoords().occlusion = mat.additionalValues["occlusionTexture"].TextureTexCoord();
			}

			if (mat.additionalValues.find("alphaMode") != mat.additionalValues.end())
			{
				tinygltf::Parameter param = mat.additionalValues["alphaMode"];

				if (param.string_value == "BLEND")
				{
					material.Alpha() = Material::ALPHAMODE_BLEND;
				}

				if (param.string_value == "MASK")
				{
					material.AlphaCutoff() = 0.5f;
					material.Alpha() = Material::ALPHAMODE_MASK;
				}
			}

			if (mat.additionalValues.find("alphaCutoff") != mat.additionalValues.end())
			{
				material.AlphaCutoff() = (float)mat.additionalValues["alphaCutoff"].Factor();
			}

			if (mat.additionalValues.find("emissiveFactor") != mat.additionalValues.end())
			{
				material.EmissiveFactor() = glm::vec4(glm::make_vec3(mat.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);
			}

			// extensions
			if (mat.extensions.find("KHR_materials_pbrSpecularGlossiness") != mat.extensions.end())
			{
				auto ext = mat.extensions.find("KHR_materials_pbrSpecularGlossiness");

				if (ext->second.Has("specularGlossinessTexture"))
				{
					auto& index = ext->second.Get("specularGlossinessTexture").Get("index");
					material.Extensions().specularGlossiness = &mTextures[index.Get<int>()];
					auto& texCoordSet = ext->second.Get("specularGlossinessTexture").Get("texCoord");
					material.TexCoords().specularGlossiness = texCoordSet.Get<int>();
					material.Workflow().specularGlossiness = true;
				}

				if (ext->second.Has("diffuseTexture"))
				{
					auto& index = ext->second.Get("diffuseTexture").Get("index");
					material.Extensions().diffuse = &mTextures[index.Get<int>()];
				}

				if (ext->second.Has("diffuseFactor"))
				{
					auto& factor = ext->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto& val = factor.Get(i);
						material.Extensions().diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}

				if (ext->second.Has("specularFactor"))
				{
					auto& factor = ext->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto& val = factor.Get(i);
						material.Extensions().specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int>();
					}
				}
			}

			mMaterials.push_back(material);
		}

		// push a default material at the end of the list for meshes with no material assigned
		mMaterials.push_back(Material());
	}

	void Model::LoadAnimations(tinygltf::Model& mdl)
	{
		for (tinygltf::Animation& anim : mdl.animations)
		{
			Animation animation = {};
			animation.Name() = anim.name;

			if (anim.name.empty())
			{
				animation.Name() = std::to_string(mAnimations.size());
			}

			// samplers
			for (auto& samp : anim.samplers)
			{
				Animation::Sampler sampler = {};

				if (samp.interpolation == "LINEAR")
				{
					sampler.interpolationType = Animation::Sampler::InterpolationType::INTERPOLATION_TYPE_LINEAR;
				}

				if (samp.interpolation == "STEP")
				{
					sampler.interpolationType = Animation::Sampler::InterpolationType::INTERPOLATION_TYPE_STEP;
				}

				if (samp.interpolation == "CUBICSPLINE")
				{
					sampler.interpolationType = Animation::Sampler::InterpolationType::INTERPOLATION_CUBIC_SPLINE;
				}

				// read sampler input time values
				{
					const tinygltf::Accessor& accessor = mdl.accessors[samp.input];
					const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];

					// assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					LOG_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Component type is not a float on animation time value");

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
					const float* buf = static_cast<const float*>(dataPtr);

					for (size_t index = 0; index < accessor.count; index++)
					{
						sampler.inputs.push_back(buf[index]);
					}

					for (auto input : sampler.inputs)
					{
						if (input < animation.GetStart())
						{
							animation.SetStart(input);
						};

						if (input > animation.GetEnd())
						{
							animation.SetEnd(input);
						}
					}
				}

				// read sampler output T/R/S values 
				{
					const tinygltf::Accessor& accessor = mdl.accessors[samp.output];
					const tinygltf::BufferView& bufferView = mdl.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = mdl.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

					switch (accessor.type)
					{
					case TINYGLTF_TYPE_VEC3:
					{
						const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);

						for (size_t index = 0; index < accessor.count; index++)
						{
							sampler.outputs.push_back(glm::vec4(buf[index], 0.0f));
						}

						break;
					}
					case TINYGLTF_TYPE_VEC4:
					{
						const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);

						for (size_t index = 0; index < accessor.count; index++)
						{
							sampler.outputs.push_back(buf[index]);
						}

						break;
					}

					default:
					{
						LOG_TO_TERMINAL(Logger::Severity::Warn, "Unknown type of accessor: %d", accessor.type);
						break;
					}
					}
				}

				animation.Samplers().push_back(sampler);
			}

			// channels
			for (auto& source : anim.channels)
			{
				Animation::Channel channel = {};

				if (source.target_path == "rotation")
				{
					channel.pathType = Animation::Channel::PathType::PATH_TYPE_ROTATION;
				}

				if (source.target_path == "translation")
				{
					channel.pathType = Animation::Channel::PathType::PATH_TYPE_TRANSLATION;
				}

				if (source.target_path == "scale")
				{
					channel.pathType = Animation::Channel::PathType::PATH_TYPE_SCALE;
				}

				if (source.target_path == "weights")
				{
					// todo: support weights
					LOG_TO_TERMINAL(Logger::Severity::Warn, "Weight is not yet supported, skipping chanel");
					continue;
				}

				channel.samplerIndex = source.sampler;
				channel.node = NodeFromIndex(source.target_node);

				if (!channel.node)
				{
					continue;
				}

				animation.Channels().push_back(channel);
			}

			mAnimations.push_back(animation);
		}
	}

	void Model::NodeProperties(const tinygltf::Node& node, const tinygltf::Model& mdl, size_t& vertexCount, size_t& indexCount)
	{
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
			{
				NodeProperties(mdl.nodes[node.children[i]], mdl, vertexCount, indexCount);
			}
		}

		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = mdl.meshes[node.mesh];

			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				auto& primitive = mesh.primitives[i];
				vertexCount += mdl.accessors[primitive.attributes.find("POSITION")->second].count;

				if (primitive.indices > -1)
				{
					indexCount += mdl.accessors[primitive.indices].count;
				}
			}
		}
	}

	VkSamplerAddressMode Model::WrapMode(int32_t mode)
	{
		switch (mode)
		{
		case -1:
		case 10497:
			return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		case 33071:
			return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		case 33648:
			return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		}

		LOG_TO_TERMINAL(Logger::Severity::Error, "Unknown wrap mode %d", mode);
		return VK_SAMPLER_ADDRESS_MODE_REPEAT;
	}

	VkFilter Model::FilterMode(int32_t mode)
	{
		switch (mode)
		{
		case -1:
		case 9728:
			return VK_FILTER_NEAREST;
		case 9729:
			return VK_FILTER_LINEAR;
		case 9984:
			return VK_FILTER_NEAREST;
		case 9985:
			return VK_FILTER_NEAREST;
		case 9986:
			return VK_FILTER_LINEAR;
		case 9987:
			return VK_FILTER_LINEAR;
		}

		LOG_TO_TERMINAL(Logger::Severity::Error, "Unknown filter mode %d", mode);
		return VK_FILTER_NEAREST;
	}

	void Model::CalculateBoundingBox(GLTFNode* node, GLTFNode* parent)
	{
		BoundingBox parentBvh = parent ? parent->BVH() : BoundingBox(mDimension.min, mDimension.max);

		if (node->GetMesh() != nullptr)
		{
			if (node->GetMesh()->BB().IsValid())
			{
				node->AABB() = node->GetMesh()->BB().AABB(node->Matrix());

				if (node->Children().size() == 0)
				{
					node->BVH().SetMin(node->AABB().GetMin());
					node->BVH().SetMax(node->AABB().GetMax());
					node->BVH().SetValid(true);
				}
			}
		}

		parentBvh.SetMin(glm::min(parentBvh.GetMin(), node->BVH().GetMin()));
		parentBvh.SetMax(glm::min(parentBvh.GetMax(), node->BVH().GetMax()));

		for (auto& child : node->Children())
		{
			CalculateBoundingBox(child, node);
		}
	}

	void Model::GetModelDimensions()
	{
		// calculate binary volume hierarchy for all nodes in the scene/model
		for (auto node : mLinearNodes)
		{
			CalculateBoundingBox(node, nullptr);
		}

		mDimension.min = glm::vec3(FLT_MAX);
		mDimension.max = glm::vec3(-FLT_MAX);

		for (auto node : mLinearNodes)
		{
			if (node->BVH().IsValid())
			{
				mDimension.min = glm::min(mDimension.min, node->BVH().GetMin());
				mDimension.max = glm::max(mDimension.max, node->BVH().GetMax());
			}
		}

		// calculate scene aabb
		mAABB = glm::scale(glm::mat4(1.0f), glm::vec3(mDimension.max[0] - mDimension.min[0], mDimension.max[1] - mDimension.min[1], mDimension.max[2] - mDimension.min[2]));
		mAABB[3][0] = mDimension.min[0];
		mAABB[3][1] = mDimension.min[1];
		mAABB[3][2] = mDimension.min[2];
	}

	GLTFNode* Model::FindNode(GLTFNode* parent, int32_t index)
	{
		GLTFNode* node = nullptr;

		if (parent->Index() == index)
		{
			return parent;
		}

		for (auto& child : parent->Children())
		{
			node = FindNode(child, index);

			if (node != nullptr)
			{
				break;
			}
		}
		return node;
	}

	GLTFNode* Model::NodeFromIndex(int32_t index)
	{
		GLTFNode* node = nullptr;

		for (auto& node : mNodes)
		{
			node = FindNode(node, index);

			if (node != nullptr)
			{
				break;
			}
		}

		return node;
	}
}