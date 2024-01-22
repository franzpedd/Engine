#include "epch.h"
#include "Model.h"

namespace Cosmos
{
	std::shared_ptr<Model> Model::Create(std::shared_ptr<Device>& device)
	{
		return std::make_shared<Model>(device);
	}

	Model::Model(std::shared_ptr<Device>& device)
		: mDevice(device)
	{
		
	}

	Model::~Model()
	{

	}

	void Model::LoadFromFile(std::string path)
	{
		mPath = path;
		
		// loads the file
		tinygltf::TinyGLTF context;
		tinygltf::Model model;
		std::string error, warning;
		
		bool loaded = context.LoadASCIIFromFile(&model, &error, &warning, path.c_str());
		
		if (!loaded)
		{
			LOG_TO_TERMINAL(Logger::Error, "Failed to load model: %s", path.c_str());
			return;
		}
		
		// loads the resources
		LoadSamplers(model);
		LoadTextures(model);
		LoadMaterials(model);
		LoadAnimations(model);
		LoadNodes(model);
	}

	void Model::LoadSamplers(tinygltf::Model& model)
	{
		// samplers
		for (tinygltf::Sampler sample : model.samplers)
		{
			TextureSampler modelSampler = {};
			modelSampler.min = TextureSampler::FilterMode(sample.minFilter);
			modelSampler.mag = TextureSampler::FilterMode(sample.magFilter);
			modelSampler.u = TextureSampler::WrapMode(sample.wrapS);
			modelSampler.v = TextureSampler::WrapMode(sample.wrapT);
			modelSampler.w = modelSampler.u;
			mSamplers.push_back(modelSampler);
		}
	}

	void Model::LoadTextures(tinygltf::Model& model)
	{
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
				sampler = mSamplers[texture.sampler];
			}
		
			std::shared_ptr<Texture2D> tex = Texture2D::Create(mDevice, image, sampler);
			mTextures.push_back(tex);
		}
	}

	void Model::LoadMaterials(tinygltf::Model& model)
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
				material.GetBaseTexture() = mTextures[gltfMaterial.values["baseColorTexture"].TextureIndex()];
				material.GetCoordinateSets().baseColor = gltfMaterial.values["baseColorTexture"].TextureTexCoord();
			}

			// metallic roughness texture
			if (gltfMaterial.values.find("metallicRoughnessTexture") != gltfMaterial.values.end())
			{
				material.GetMetallicRoughnessTexture() = mTextures[gltfMaterial.values["metallicRoughnessTexture"].TextureIndex()];
				material.GetCoordinateSets().metallicRoughness = gltfMaterial.values["metallicRoughnessTexture"].TextureTexCoord();
			}

			// normal map texture
			if (gltfMaterial.additionalValues.find("normalTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetNormalMap() = mTextures[gltfMaterial.additionalValues["normalTexture"].TextureIndex()];
				material.GetCoordinateSets().normalMap = gltfMaterial.additionalValues["normalTexture"].TextureTexCoord();
			}

			// occlusion map texture
			if (gltfMaterial.additionalValues.find("occlusionTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetOcclusionMap() = mTextures[gltfMaterial.additionalValues["occlusionTexture"].TextureIndex()];
				material.GetCoordinateSets().occlusionMap = gltfMaterial.additionalValues["occlusionTexture"].TextureTexCoord();
			}

			// emissive map texture 
			if (gltfMaterial.additionalValues.find("emissiveTexture") != gltfMaterial.additionalValues.end())
			{
				material.GetEmissiveMap() = mTextures[gltfMaterial.additionalValues["emissiveTexture"].TextureIndex()];
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
					material.GetSpecularGlossiness().specularGlossiness = mTextures[index.Get<int32_t>()];
					auto& texCoordSet = extension->second.Get("specularGlossinessTexture").Get("texCoord");
					material.GetCoordinateSets().specularGlossiness = texCoordSet.Get<int32_t>();
					material.GetWorkflow() = Material::Workflow::SPECULAR_GLOSSINESS;
				}

				// diffuse texture
				if (extension->second.Has("diffuseTexture"))
				{
					auto& index = extension->second.Get("diffuseTexture").Get("index");
					material.GetSpecularGlossiness().diffuseMap = mTextures[index.Get<int32_t>()];
				}
			}

			mMaterials.push_back(material);
		}

		mMaterials.push_back(Material()); // empty material at the end for meshes without any material assigned
	}

	void Model::LoadAnimations(tinygltf::Model& model)
	{
		if (model.animations.size() == 0)
			return;

		for (tinygltf::Animation& animation : model.animations)
		{

		}
	}

	void Model::LoadNodes(tinygltf::Model& model)
	{
		// get vertex and index buffer sizes up-front
		const tinygltf::Scene& scene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
		
		LoaderInfo info = {};
		size_t vertexCount = 0, indexCount = 0;
		
		for (size_t i = 0; i < scene.nodes.size(); i++)
			GetNodeProperties(model, model.nodes[scene.nodes[i]], vertexCount, indexCount);
		
		info.vertexBuffer = new Vertex[vertexCount];
		info.indexBuffer = new uint32_t[indexCount];
		
		LOG_TO_TERMINAL(Logger::Warn, "Handle models without default scene");
		
		for (size_t i = 0; i < scene.nodes.size(); i++)
		{
			const tinygltf::Node node = model.nodes[scene.nodes[i]];
			LoadNode(nullptr, node, scene.nodes[i], model, info);
		} 
	}

	void Model::GetNodeProperties(tinygltf::Model& model, tinygltf::Node& node, size_t& vertexCount, size_t& indexCount)
	{
		if (node.children.size() > 0)
		{
			for (size_t i = 0; i < node.children.size(); i++)
				GetNodeProperties(model, model.nodes[node.children[i]], vertexCount, indexCount);
		}

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

	void Model::LoadNode(ModelNode* parent, const tinygltf::Node& node, uint32_t nodeIndex, const tinygltf::Model& model, LoaderInfo& loaderInfo)
	{
		ModelNode* modelNode = new ModelNode();
		modelNode->GetIndex() = nodeIndex;
		modelNode->GetParent() = parent;
		modelNode->GetName() = node.name;
		modelNode->GetSkinIndex() = node.skin;
		modelNode->GetMatrix() = glm::mat4(1.0f);

		// generate local node matrix
		LOG_TO_TERMINAL(Logger::Warn, "Check the rotation quaternion and transform it to mat4 if causes bug");

		if (node.translation.size() == 3)
			modelNode->GetTranslation() = glm::make_vec3(node.translation.data());

		if (node.rotation.size() == 4)
			modelNode->GetRotation() = glm::make_quat(node.rotation.data());

		if (node.scale.size() == 3)
			modelNode->GetScale() = glm::make_vec3(node.scale.data());

		if (node.matrix.size() == 16)
			modelNode->GetMatrix() = glm::make_mat4x4(node.matrix.data());

		// node has children, must create their nodes as well
		LOG_TO_TERMINAL(Logger::Warn, "Move children recursive function to the end if easy to be done");

		if (node.children.size() > 0)
			for (size_t i = 0; i < node.children.size(); i++)
				LoadNode(modelNode, model.nodes[node.children[i]], node.children[i], model, loaderInfo);

		// load nodes meshes if it has meshes to be loaded
		LoadNodeMeshes(modelNode, node, model, loaderInfo);
	}

	void Model::LoadNodeMeshes(ModelNode* modelNode, const tinygltf::Node& node, const tinygltf::Model& model, LoaderInfo& info)
	{
		// node contains meshes
		if (node.mesh > -1)
		{
			const tinygltf::Mesh mesh = model.meshes[node.mesh];
			ModelMesh* modelMesh = new ModelMesh(mDevice, modelNode->GetMatrix());

			for (size_t i = 0; i < mesh.primitives.size(); i++)
			{
				const tinygltf::Primitive& primitive = mesh.primitives[i];
				uint32_t vertexStart = (uint32_t)info.vertexPosition;
				uint32_t indexStart = (uint32_t)info.indexPosition;
				uint32_t vertexCount = 0;
				uint32_t indexCount = 0;
				glm::vec3 positionMin = {};
				glm::vec3 positionMax = {};
				bool hasSkin = false;
				bool hasIndices = primitive.indices > 1;

				// handle meshe's vertices
				const float* position = nullptr;
				const float* normals = nullptr;
				const float* uv0 = nullptr;
				const float* uv1 = nullptr;
				const float* color = nullptr;
				const float* weights = nullptr;
				const void* joints = nullptr;

				uint32_t positionStride = 0;
				uint32_t normalsStride = 0;
				uint32_t uv0Stride = 0;
				uint32_t uv1Stride = 0;
				uint32_t colorStride = 0;
				uint32_t weightsStride = 0;
				uint32_t jointsStride = 0;

				uint32_t jointType = 0;

				// position (it is required to have a position, otherwise it'll crash)

				if (!(primitive.attributes.find("POSITION") != primitive.attributes.end()))
				{
					LOG_TO_TERMINAL(Logger::Error, "Model has no position and should be deleted");
				}

				const tinygltf::Accessor& posAccessor = model.accessors[primitive.attributes.find("POSITION")->second];
				const tinygltf::BufferView& posView = model.bufferViews[posAccessor.bufferView];

				position = reinterpret_cast<const float*>(&(model.buffers[posView.buffer].data[posAccessor.byteOffset + posView.byteOffset]));
				positionMin = glm::vec3(posAccessor.minValues[0], posAccessor.minValues[1], posAccessor.minValues[2]);
				positionMax = glm::vec3(posAccessor.maxValues[0], posAccessor.maxValues[1], posAccessor.maxValues[2]);
				vertexCount = static_cast<uint32_t>(posAccessor.count);
				positionStride = posAccessor.ByteStride(posView) ? (posAccessor.ByteStride(posView) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);

				// normals
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

				// color
				if (primitive.attributes.find("COLOR_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& accessor = model.accessors[primitive.attributes.find("COLOR_0")->second];
					const tinygltf::BufferView& view = model.bufferViews[accessor.bufferView];
					color = reinterpret_cast<const float*>(&(model.buffers[view.buffer].data[accessor.byteOffset + view.byteOffset]));
					colorStride = accessor.ByteStride(view) ? (accessor.ByteStride(view) / sizeof(float)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC3);
				}

				// joints
				if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end())
				{
					const tinygltf::Accessor& jointAccessor = model.accessors[primitive.attributes.find("JOINTS_0")->second];
					const tinygltf::BufferView& jointView = model.bufferViews[jointAccessor.bufferView];
					joints = &(model.buffers[jointView.buffer].data[jointAccessor.byteOffset + jointView.byteOffset]);
					jointType = jointAccessor.componentType;
					jointsStride = jointAccessor.ByteStride(jointView) ? (jointAccessor.ByteStride(jointView) / tinygltf::GetComponentSizeInBytes(jointType)) : tinygltf::GetNumComponentsInType(TINYGLTF_TYPE_VEC4);
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

				for (size_t v = 0; v < posAccessor.count; v++)
				{
					Vertex& vert = info.vertexBuffer[info.vertexPosition];
					vert.position = glm::vec4(glm::make_vec3(&position[v * positionStride]), 1.0f);
					vert.normal = glm::normalize(glm::vec3(normals ? glm::make_vec3(&normals[v * normalsStride]) : glm::vec3(0.0f)));
					vert.uv0 = uv0 ? glm::make_vec2(&uv0[v * uv0Stride]) : glm::vec3(0.0f);
					vert.uv1 = uv1 ? glm::make_vec2(&uv1[v * uv1Stride]) : glm::vec3(0.0f);
					vert.color = color ? glm::make_vec4(&color[v * colorStride]) : glm::vec4(1.0f);
					vert.joint = glm::vec4(0.0f);
					vert.weight = hasSkin ? glm::make_vec4(&weights[v * weightsStride]) : glm::vec4(0.0f);

					// fix for all zero weights
					if (glm::length(vert.weight) == 0.0f)
						vert.weight = glm::vec4(1.0f, 0.0f, 0.0f, 0.0f);

					if (hasSkin)
					{
						switch (jointType)
						{
						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
						{
							const uint16_t* buf = (const uint16_t*)joints;
							vert.joint = glm::vec4(glm::make_vec4(&buf[v * jointsStride]));
							break;
						}

						case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
						{
							const uint8_t* buf = (const uint8_t*)joints;
							vert.joint = glm::vec4(glm::make_vec4(&buf[v * jointsStride]));
							break;
						}

						default:
						{
							LOG_TO_TERMINAL(Logger::Error, "Joint component type %d is not supported", jointType);
							break;
						}
						}
					}

					info.vertexPosition++;
				}

				// handle meshe's indices
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
			}
		}
	}

	ModelPrimitive::ModelPrimitive(Material& material, uint32_t vertexCount, uint32_t indexCount, uint32_t firstIndex)
		: mMaterial(material), mVertexCount(vertexCount), mIndexCount(indexCount), mFirstIndex(firstIndex)
	{
		mHasIndices = indexCount > 0 ? true : false;
	}

	void ModelPrimitive::SetBoundingBox(glm::vec3 min, glm::vec3 max)
	{
		mBB.SetMin(min);
		mBB.SetMax(max);
		mBB.SetValid(true);
	}

	ModelNode::~ModelNode()
	{
		if (mMesh != nullptr)
			delete mMesh;

		for (auto& child : mChildren)
			delete child;
	}
}