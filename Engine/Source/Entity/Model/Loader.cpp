#include "epch.h"
#include "Loader.h"

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"

namespace Cosmos::ModelHelper
{
	Loader::Loader(std::shared_ptr<Renderer>& renderer, std::string& path)
		: renderer(renderer), path(path)
	{
		Info info = {};
		size_t vertexCount = 0;
		size_t indexCount = 0;
		tinygltf::TinyGLTF context;
		std::string error, warning;

		bool loaded = context.LoadASCIIFromFile(&model, &error, &warning, path.c_str());
		
		if (!loaded)
		{
			LOG_TO_TERMINAL(Logger::Error, "Failed to load model: %s", path.c_str());
			return;
		}

		LoadTextures();
		LoadMaterials();
		LoadNodes(info, vertexCount, indexCount);
		LoadAnimations();
		LoadSkins();
		CreateRendererResources(info, vertexCount, indexCount);
	}

	void Loader::Destroy()
	{
		if (vertexBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(renderer->GetDevice()->GetDevice(), vertexBuffer, nullptr);
			vkFreeMemory(renderer->GetDevice()->GetDevice(), vertexMemory, nullptr);
			vertexBuffer = VK_NULL_HANDLE;
			vertexMemory = VK_NULL_HANDLE;
		}
		
		if (indexBuffer != VK_NULL_HANDLE)
		{
			vkDestroyBuffer(renderer->GetDevice()->GetDevice(), indexBuffer, nullptr);
			vkFreeMemory(renderer->GetDevice()->GetDevice(), indexMemory, nullptr);
			vertexBuffer = VK_NULL_HANDLE;
			vertexMemory = VK_NULL_HANDLE;
		}
		
		for (auto& tex : textures)
			tex->Destroy();
		
		// mTextures.resize(0) breaks as there's no default constructor
		textures.clear();
		samplers.resize(0);
		
		for (auto& node : nodes)
			delete node;
		
		materials.resize(0);
		animations.resize(0);
		linearNodes.resize(0);
		
		for (auto& skin : skins)
			delete skin;
		
		skins.resize(0);
	}

	void Loader::LoadTextures()
	{
		for (tinygltf::Sampler sample : model.samplers)
		{
			TextureSampler modelSampler = {};
			modelSampler.min = TextureSampler::FilterMode(sample.minFilter);
			modelSampler.mag = TextureSampler::FilterMode(sample.magFilter);
			modelSampler.u = TextureSampler::WrapMode(sample.wrapS);
			modelSampler.v = TextureSampler::WrapMode(sample.wrapT);
			modelSampler.w = modelSampler.u;
			samplers.push_back(modelSampler);
		}

		for (tinygltf::Texture& texture : model.textures)
		{
			tinygltf::Image image = model.images[texture.source];
			TextureSampler sampler = {};
		
			if (texture.sampler == -1) // default filter and address mode if not previously defined
			{
				sampler.mag = TextureSampler::Filter::FILTER_LINEAR;
				sampler.min = TextureSampler::Filter::FILTER_LINEAR;
				sampler.u = TextureSampler::AddressMode::ADDRESS_MODE_MIRRORED_REPEAT;
				sampler.v = TextureSampler::AddressMode::ADDRESS_MODE_MIRRORED_REPEAT;
				sampler.w = TextureSampler::AddressMode::ADDRESS_MODE_MIRRORED_REPEAT;
			}
		
			else
			{
				sampler = samplers[texture.sampler];
			}
		
			std::shared_ptr<Texture2D> tex = Texture2D::Create(renderer->GetDevice(), image, sampler);
			textures.push_back(tex);
		}
	}

	void Loader::LoadMaterials()
	{
		for (tinygltf::Material& gltfMaterial : model.materials)
		{
			Material material = {};

			// the order of search matches with Material private members in order to facilitate future extensions

			// alpha mode
			if (gltfMaterial.additionalValues.find("alphaMode") != gltfMaterial.additionalValues.end())
			{
				tinygltf::Parameter param = gltfMaterial.additionalValues["alphaMode"];

				if (param.string_value == "BLEND")
					material.GetAlphaMode() = Material::AlphaMode::ALPHAMODE_BLEND;

				if (param.string_value == "MASK")
				{
					material.GetAlphaCutoff() = 0.5f;
					material.GetAlphaMode() = Material::AlphaMode::ALPHAMODE_MASK;
				}
			}

			// alpha cutoff
			if (gltfMaterial.additionalValues.find("alphaCutoff") != gltfMaterial.additionalValues.end())
				material.GetAlphaCutoff() = (float)gltfMaterial.additionalValues["alphaCutoff"].Factor();

			// metallic factor
			if (gltfMaterial.values.find("metallicFactor") != gltfMaterial.values.end())
				material.GetMetallicFactor() = (float)gltfMaterial.values["metallicFactor"].Factor();

			// roughness factor
			if (gltfMaterial.values.find("roughnessFactor") != gltfMaterial.values.end())
				material.GetRoughnessFactor() = (float)gltfMaterial.values["roughnessFactor"].Factor();

			// base color factor
			if (gltfMaterial.values.find("baseColorFactor") != gltfMaterial.values.end())
				material.GetBaseColorFactor() = glm::make_vec4(gltfMaterial.values["baseColorFactor"].ColorFactor().data());

			// emissive factor
			if (gltfMaterial.additionalValues.find("emissiveFactor") != gltfMaterial.additionalValues.end())
				material.GetEmissiveFactor() = glm::vec4(glm::make_vec3(gltfMaterial.additionalValues["emissiveFactor"].ColorFactor().data()), 1.0);

			// base color texture
			if (gltfMaterial.values.find("baseColorTexture") != gltfMaterial.values.end())
			{
				material.GetBaseTexture() = textures[gltfMaterial.values["baseColorTexture"].TextureIndex()];
				material.GetCoordinateSets().baseColor = gltfMaterial.values["baseColorTexture"].TextureTexCoord();
			}

			// metallic roughness texture
			if (gltfMaterial.values.find("metallicRoughnessTexture") != gltfMaterial.values.end())
			{
				material.GetMetallicRoughnessTexture() = textures[gltfMaterial.values["metallicRoughnessTexture"].TextureIndex()];
				material.GetCoordinateSets().metallicRoughness = gltfMaterial.values["metallicRoughnessTexture"].TextureTexCoord();
			}

			// normal map texture
			if (gltfMaterial.additionalValues.find("normalTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetNormalMap() = textures[gltfMaterial.additionalValues["normalTexture"].TextureIndex()];
				material.GetCoordinateSets().normalMap = gltfMaterial.additionalValues["normalTexture"].TextureTexCoord();
			}

			// occlusion map texture
			if (gltfMaterial.additionalValues.find("occlusionTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetOcclusionMap() = textures[gltfMaterial.additionalValues["occlusionTexture"].TextureIndex()];
				material.GetCoordinateSets().occlusionMap = gltfMaterial.additionalValues["occlusionTexture"].TextureTexCoord();
			}

			// emissive map texture 
			if (gltfMaterial.additionalValues.find("emissiveTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetEmissiveMap() = textures[gltfMaterial.additionalValues["emissiveTexture"].TextureIndex()];
				material.GetCoordinateSets().emissiveMap = gltfMaterial.additionalValues["emissiveTexture"].TextureTexCoord();
			}

			// specular glossiness extension
			if (gltfMaterial.extensions.find("KHR_materials_pbrSpecularGlossiness") != gltfMaterial.extensions.end())
			{
				auto extension = gltfMaterial.extensions.find("KHR_materials_pbrSpecularGlossiness");

				// specular factor
				if (extension->second.Has("specularFactor"))
				{
					auto& factor = extension->second.Get("specularFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto& val = factor.Get(i);
						material.GetSpecularGlossiness().specularFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int32_t>();
					}
				}

				// diffuse factor
				if (extension->second.Has("diffuseFactor"))
				{
					auto& factor = extension->second.Get("diffuseFactor");
					for (uint32_t i = 0; i < factor.ArrayLen(); i++)
					{
						auto& val = factor.Get(i);
						material.GetSpecularGlossiness().diffuseFactor[i] = val.IsNumber() ? (float)val.Get<double>() : (float)val.Get<int32_t>();
					}
				}

				// specular glossiness texture
				if (extension->second.Has("specularGlossinessTexture"))
				{
					auto& index = extension->second.Get("specularGlossinessTexture").Get("index");
					material.GetSpecularGlossiness().specularGlossiness = textures[index.Get<int32_t>()];
					auto& texCoordSet = extension->second.Get("specularGlossinessTexture").Get("texCoord");
					material.GetCoordinateSets().specularGlossiness = texCoordSet.Get<int32_t>();
					material.GetWorkflow() = Material::Workflow::SPECULAR_GLOSSINESS;
				}

				// diffuse texture
				if (extension->second.Has("diffuseTexture"))
				{
					auto& index = extension->second.Get("diffuseTexture").Get("index");
					material.GetSpecularGlossiness().diffuseMap = textures[index.Get<int32_t>()];
				}
			}

			materials.push_back(material);
		}

		// empty material at the end for meshes without any material assigned
		materials.push_back(Material());
	}

	void Loader::LoadNodes(Info& info, size_t& vertexCount, size_t& indexCount)
	{
		const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
		
		for (size_t i = 0; i < scene.nodes.size(); i++)
			GetNodeProperties(model, model.nodes[scene.nodes[i]], vertexCount, indexCount);
		
		info.vertexBuffer = new Vertex[vertexCount];
		info.indexBuffer = new uint32_t[indexCount];
		
		LOG_TO_TERMINAL(Logger::Todo, "Model: Handle scene with no default scene");
		
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			const tinygltf::Node node = model.nodes[scene.nodes[i]];
			CreateNode(nullptr, node, scene.nodes[i], info);
		}
	}

	void Loader::LoadAnimations()
	{
		if (model.animations.size() == 0)
			return;

		for (tinygltf::Animation& anim : model.animations)
		{
			Animation animation = {};
			animation.name = anim.name;

			if (anim.name.empty())
				animation.name = std::to_string(animations.size());

			// samplers
			for (auto& samp : anim.samplers)
			{
				Animation::Sampler sampler = {};

				if (samp.interpolation == "LINEAR")
					sampler.interpolationType = Animation::Sampler::InterpolationType::LINEAR;

				if (samp.interpolation == "STEP")
					sampler.interpolationType = Animation::Sampler::InterpolationType::STEP;

				if (samp.interpolation == "CUBICSPLINE")
					sampler.interpolationType = Animation::Sampler::InterpolationType::CUBIC_SPLINE;

				// read sampler input time values
				{
					const tinygltf::Accessor& accessor = model.accessors[samp.input];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					// assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);
					LOG_ASSERT(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Component type is not a float on animation time value");

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];
					const float* buf = static_cast<const float*>(dataPtr);

					for (size_t index = 0; index < accessor.count; index++)
						sampler.inputs.push_back(buf[index]);

					for (auto input : sampler.inputs)
					{
						if (input < animation.start)
							animation.start = input;

						if (input > animation.end)
							animation.end = input;
					}
				}

				// read sampler output T/R/S values 
				{
					const tinygltf::Accessor& accessor = model.accessors[samp.output];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					assert(accessor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT);

					const void* dataPtr = &buffer.data[accessor.byteOffset + bufferView.byteOffset];

					switch (accessor.type)
					{
					case TINYGLTF_TYPE_VEC3:
					{
						const glm::vec3* buf = static_cast<const glm::vec3*>(dataPtr);

						for (size_t index = 0; index < accessor.count; index++)
							sampler.outputs.push_back(glm::vec4(buf[index], 0.0f));

						break;
					}
					case TINYGLTF_TYPE_VEC4:
					{
						const glm::vec4* buf = static_cast<const glm::vec4*>(dataPtr);

						for (size_t index = 0; index < accessor.count; index++)
							sampler.outputs.push_back(buf[index]);

						break;
					}

					default:
					{
						LOG_TO_TERMINAL(Logger::Severity::Warn, "Unknown type of accessor: %d", accessor.type);
						break;
					}
					}
				}

				animation.samplers.push_back(sampler);
			}

			// channels
			for (auto& source : anim.channels)
			{
				Animation::Channel channel = {};

				if (source.target_path == "rotation")
					channel.pathType = Animation::Channel::PathType::ROTATION;

				if (source.target_path == "translation")
					channel.pathType = Animation::Channel::PathType::TRANSLATION;

				if (source.target_path == "scale")
					channel.pathType = Animation::Channel::PathType::SCALE;

				if (source.target_path == "weights")
				{
					// todo: support weights
					LOG_TO_TERMINAL(Logger::Severity::Warn, "Weight is not yet supported, skipping chanel");
					continue;
				}

				channel.samplerIndex = source.sampler;
				channel.node = FindNodeByIndex(source.target_node);

				if (!channel.node)
				{
					continue;
				}

				animation.channels.push_back(channel);
			}

			animations.push_back(animation);
		}
	}

	void Loader::LoadSkins()
	{
		for (tinygltf::Skin& source : model.skins)
		{
			Skin* newSkin = new Skin();
			newSkin->name = source.name;

			// find skeleton root node
			if (source.skeleton > -1)
				newSkin->root = FindNodeByIndex(source.skeleton);

			// find joint nodes
			for (int jointIndex : source.joints)
			{
				Node* node = FindNodeByIndex(jointIndex);

				if (node != nullptr)
					newSkin->joints.push_back(FindNodeByIndex(jointIndex));
			}

			// get inverse bind matrices from buffer
			if (source.inverseBindMatrices > -1)
			{
				const tinygltf::Accessor& accessor = model.accessors[source.inverseBindMatrices];
				const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
				const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

				newSkin->inverseMatrices.resize(accessor.count);
				memcpy(newSkin->inverseMatrices.data(), &buffer.data[accessor.byteOffset + bufferView.byteOffset], accessor.count * sizeof(glm::mat4));
			}

			skins.push_back(newSkin);
		}

		// assign skins and update to it's initial pose
		for (auto node : linearNodes)
		{
			if (node->skinIndex > -1)
				node->skin = skins[node->skinIndex];

			if (node->mesh)
				node->Update();
		}
	}

	void Loader::CreateRendererResources(Info& info, size_t& vertexCount, size_t& indexCount)
	{
		size_t vertexBufferSize = vertexCount * sizeof(Vertex);
		size_t indexBufferSize = indexCount * sizeof(uint32_t);

		if (vertexBufferSize <= 0)
			LOG_TO_TERMINAL(Logger::Error, "Model: Something went wront and the model doesn't contain vertices");

		// create staging buffers
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
				renderer->GetDevice(),
				VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
				(VkDeviceSize)vertexBufferSize,
				&vertexStaging.buffer,
				&vertexStaging.memory,
				info.vertexBuffer
			), "Failed to create Staging Vertex Buffer"
		);

		// staging index buffer
		if (indexBufferSize > 0)
		{
			VK_ASSERT
			(
				BufferCreate
				(
					renderer->GetDevice(),
					VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
					VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
					(VkDeviceSize)indexBufferSize,
					&indexStaging.buffer,
					&indexStaging.memory,
					info.indexBuffer
				), "Failed to create Staging Index Buffer"
			);
		}

		// gpu vertex buffer
		VK_ASSERT
		(
			BufferCreate
			(
				renderer->GetDevice(),
				VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				(VkDeviceSize)vertexBufferSize,
				&vertexBuffer,
				&vertexMemory
			), "Failed to create GPU Vertex Buffer"
		);

		// gpu index buffer (if has any)
		if (indexBufferSize > 0)
		{
			VK_ASSERT
			(
				BufferCreate
				(
					renderer->GetDevice(),
					VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					VK_BUFFER_USAGE_TRANSFER_DST_BIT,
					(VkDeviceSize)indexBufferSize,
					&indexBuffer,
					&indexMemory
				), "Failed to create GPU Index Buffer"
			);
		}

		LOG_TO_TERMINAL(Logger::Todo, "Create custom command entry for models.");

		// copy from staging buffers
		VkCommandBuffer copyCmd = CreateCommandBuffer(std::reinterpret_pointer_cast<VKDevice>(renderer->GetDevice()), renderer->GetDevice()->GetMainCommandEntry()->commandPool, VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

		VkBufferCopy copyRegion = {};
		copyRegion.size = vertexBufferSize;
		copyRegion.dstOffset = 0;
		copyRegion.srcOffset = 0;
		vkCmdCopyBuffer(copyCmd, vertexStaging.buffer, vertexBuffer, 1, &copyRegion);

		if (indexBufferSize > 0)
		{
			copyRegion.dstOffset = 0;
			copyRegion.srcOffset = 0;
			copyRegion.size = indexBufferSize;
			vkCmdCopyBuffer(copyCmd, indexStaging.buffer, indexBuffer, 1, &copyRegion);
		}

		FlushCommandBuffer(std::reinterpret_pointer_cast<VKDevice>(renderer->GetDevice()), renderer->GetDevice()->GetMainCommandEntry()->commandPool, copyCmd, renderer->GetDevice()->GetGraphicsQueue(), true);

		// free staging buffers
		vkDestroyBuffer(renderer->GetDevice()->GetDevice(), vertexStaging.buffer, nullptr);
		vkFreeMemory(renderer->GetDevice()->GetDevice(), vertexStaging.memory, nullptr);

		if (indexBufferSize > 0)
		{
			vkDestroyBuffer(renderer->GetDevice()->GetDevice(), indexStaging.buffer, nullptr);
			vkFreeMemory(renderer->GetDevice()->GetDevice(), indexStaging.memory, nullptr);
		}
		
		delete[] info.vertexBuffer;
		delete[] info.indexBuffer;
	}

	void Loader::CreateNode(Node* parent, const tinygltf::Node& node, uint32_t nodeIndex, Info& info)
	{
		Node* newNode = new Node();
		newNode->index = nodeIndex;
		newNode->parent = parent;
		newNode->name = node.name;
		newNode->skinIndex = node.skin;
		newNode->matrix = glm::mat4(1.0f);
		
		// generate local node matrix
		glm::vec3 translation = glm::vec3(0.0f);
		if (node.translation.size() == 3)
		{
			translation = glm::make_vec3(node.translation.data());
			newNode->translation = translation;
		}
		
		glm::mat4 rotation = glm::mat4(1.0f);
		if (node.rotation.size() == 4)
		{
			glm::quat q = glm::make_quat(node.rotation.data());
			newNode->rotation = glm::mat4(q);
		}
		
		glm::vec3 scale = glm::vec3(1.0f);
		if (node.scale.size() == 3)
		{
			scale = glm::make_vec3(node.scale.data());
			newNode->scale = scale;
		}
		
		if (node.matrix.size() == 16)
			newNode->matrix = glm::make_mat4x4(node.matrix.data());
		
		// node with children
		if (node.children.size() > 0)
			for (size_t i = 0; i < node.children.size(); i++)
				CreateNode(newNode, model.nodes[node.children[i]], node.children[i], info);

		// node with mesh
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			Mesh* newMesh = new Mesh(renderer, newNode->matrix);

			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				const tinygltf::Primitive& primitive = mesh.primitives[i];
				uint32_t vertexStart = (uint32_t)info.vertexPosition;
				uint32_t indexStart = (uint32_t)info.indexPosition;
				uint32_t indexCount = 0;
				uint32_t vertexCount = 0;
				glm::vec3 posMin = {};
				glm::vec3 posMax = {};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > -1;

				// vertices
				const float* position = nullptr;
				const float* normals = nullptr;
				const float* uv0 = nullptr;
				const float* uv1 = nullptr;
				const float* color0 = nullptr;
				const void* joints = nullptr;
				const float* weights = nullptr;

				int32_t positionStride;
				int32_t normalsStride;
				int32_t uv0Stride;
				int32_t uv1Stride;
				int32_t color0Stride;
				int32_t jointsStride;
				int32_t weightsStride;

				int32_t jointComponentType;

				if (primitive.attributes.find("POSITION") == primitive.attributes.end())
				{
					LOG_TO_TERMINAL(Logger::Error, "Position is required for loading a model's mesh");
				}

				// position
				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];

				position = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				posMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				posMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
				vertexCount = static_cast<uint32_t>(posAccessor.count);
				positionStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

				// normal maps
				if (primitive.attributes.find("NORMAL") != primitive.attributes.end())
				{
					const tinygltf::Accessor& normAccessor = model.accessors[primitive.attributes.find("NORMAL")->second];
					const tinygltf::BufferView& normView = model.bufferViews[normAccessor.bufferView];
					normals = reinterpret_cast<const float*>(&(model.buffers[normView.buffer].data[normAccessor.byteOffset + normView.byteOffset]));
					normalsStride = normAccessor.ByteStride(normView) ? (normAccessor.ByteStride(normView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				}

				// uv0
				if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_0")->second];
					const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
					uv0 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					uv0Stride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
				}

				// uv1
				if (primitive.attributes.find("TEXCOORD_1") != primitive.attributes.end())
				{
					const tinygltf::Accessor& uvAccessor = model.accessors[primitive.attributes.find("TEXCOORD_1")->second];
					const tinygltf::BufferView& uvView = model.bufferViews[uvAccessor.bufferView];
					uv1 = reinterpret_cast<const float*>(&(model.buffers[uvView.buffer].data[uvAccessor.byteOffset + uvView.byteOffset]));
					uv1Stride = uvAccessor.ByteStride(uvView) ? (uvAccessor.ByteStride(uvView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC2);
				}

				// color0
				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					color0 = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					color0Stride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				}

				// joints
				if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
					joints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
					jointComponentType = jointAccessor.componentType;
					jointsStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointComponentType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
				}

				// weights
				if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& weightAccessor = model.accessors[primitive.attributes.find("WEIGHTS_0")->second];
					const tinygltf::BufferView& weightView = model.bufferViews[weightAccessor.bufferView];
					weights = reinterpret_cast<const float*>(&(model.buffers[weightView.buffer].data[weightAccessor.byteOffset + weightView.byteOffset]));
					weightsStride = weightAccessor.ByteStride(weightView) ? (weightAccessor.ByteStride(weightView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
				}

				hasSkin = (joints && weights);

				for (size_t j = 0; j < posAccessor.count; j++)
				{
					Vertex& vert = info.vertexBuffer[info.vertexPosition];
					vert.position = glm::vec4(glm::make_vec3(&position[j * positionStride]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(normals ? glm::make_vec3(&normals[j * normalsStride]) : glm::vec3(0.0f)));
					vert.uv0 = uv0 ? glm::make_vec2(&uv0[j * uv0Stride]) : glm::vec3(0.0f);
					vert.uv1 = uv1 ? glm::make_vec2(&uv1[j * uv1Stride]) : glm::vec3(0.0f);
					vert.color = color0 ? glm::make_vec4(&color0[j * color0Stride]) : glm::vec4(1.0f);

					if (hasSkin)
					{
						switch (jointComponentType)
						{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = (const uint16_t*)joints;
							vert.joint = glm::vec4(glm::make_vec4(&buf[j * jointsStride]));
							break;
						}

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf = (const uint8_t*)joints;
							vert.joint = glm::vec4(glm::make_vec4(&buf[j * jointsStride]));
							break;
						}

						default:
						{
							LOG_TO_TERMINAL(Logger::Severity::Error, "Joint component type %d is not supported", jointComponentType);
							break;
						}
						}
					}

					else
					{
						vert.joint = glm::vec4(0.0f);
					}

					vert.weight = hasSkin ? glm::make_vec4(&weights[j * weightsStride]) : glm::vec4(0.0f);

					// fix for all zero weights
					if (glm::length(vert.weight) == 0.0f)
						vert.weight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

					info.vertexPosition++;
				}

				// indices
				if (hasIndices)
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.indices > -1 ? primitive.indices : 0];
					const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
					const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];

					indexCount = (uint32_t)accessor.count;
					const void* dataPtr = &(buffer.data[accessor.byteOffset + bufferView.byteOffset]);

					switch (accessor.componentType)
					{
					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT:
					{
						const uint32_t* buf = (const uint32_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							info.indexBuffer[info.indexPosition] = buf[index] + vertexStart;
							info.indexPosition++;
						}

						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT:
					{
						const uint16_t* buf = (const uint16_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							info.indexBuffer[info.indexPosition] = buf[index] + vertexStart;
							info.indexPosition++;
						}

						break;
					}

					case TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE:
					{
						const uint8_t* buf = (const uint8_t*)dataPtr;

						for (size_t index = 0; index < accessor.count; index++)
						{
							info.indexBuffer[info.indexPosition] = buf[index] + vertexStart;
							info.indexPosition++;
						}

						break;
					}

					default:
					{
						LOG_TO_TERMINAL(Logger::Severity::Error, "Index component type %d is not supported", accessor.componentType);
						return;
					}
					}
				}

				Material& mat = primitive.material > -1 ? materials[primitive.material] : materials.back();
				Primitive* newPrimitive = new Primitive(mat, indexStart, indexCount, vertexCount);
				newPrimitive->SetBoundingBox(posMin, posMax);

				newMesh->primitives.push_back(newPrimitive);
			}

			// mesh BB from BBs of primitives
			for (auto p : newMesh->primitives)
			{
				if (p->bb.IsValid() && !newMesh->bb.IsValid())
				{
					newMesh->bb = p->bb;
					newMesh->bb.SetValid(true);
				}

				newMesh->bb.SetMin(glm::min(newMesh->bb.GetMin(), p->bb.GetMin()));
				newMesh->bb.SetMax(glm::max(newMesh->bb.GetMax(), p->bb.GetMax()));
			}

			newNode->mesh = newMesh;
		}

		if (parent)
			parent->children.push_back(newNode);

		else
			nodes.push_back(newNode);

		linearNodes.push_back(newNode);
	}

	Node* Loader::FindChildNode(Node* parent, int32_t index)
	{
		Node* node = nullptr;
		
		if (parent->index == index)
			return parent;
		
		for (auto& child : parent->children)
		{
			node = FindChildNode(child, index);
		
			if (node != nullptr)
				break;
		}
		
		return node;
	}

	Node* Loader::FindNodeByIndex(int32_t index)
	{
		Node* node = nullptr;
		
		for (auto& node : nodes)
		{
			node = FindChildNode(node, index);
		
			if (node != nullptr)
				break;
		}
		
		return node;
	}

	void Loader::GetNodeProperties(tinygltf::Model& model, tinygltf::Node& node, size_t& vertexCount, size_t& indexCount)
	{
		if (node.children.size() > 0)
			for (size_t i = 0; i < node.children.size(); i++)
				GetNodeProperties(model, model.nodes[node.children[i]], vertexCount, indexCount);
		
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
		
			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				auto& primitive = mesh.primitives[i];
				vertexCount += model.accessors[primitive.attributes.find("POSITION")->second].count;
		
				if (primitive.indices > -1)
				{
					indexCount += model.accessors[primitive.indices].count;
				}
			}
		}
	}
}