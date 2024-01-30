#include "epch.h"
#include "MeshLoader.h"

#include "Renderer/Renderer.h"
#include "Renderer/Vulkan/VKBuffer.h"

namespace Cosmos
{
	MeshLoader::MeshLoader(std::shared_ptr<Renderer>& renderer)
		: mRenderer(renderer)
	{
	}

	MeshLoader::~MeshLoader()
	{
		mEntries.clear();
	}

	void MeshLoader::LoadMesh(std::string path)
	{
		int32_t flags = aiProcess_FlipWindingOrder 
			| aiProcess_Triangulate 
			| aiProcess_PreTransformVertices 
			| aiProcess_CalcTangentSpace 
			| aiProcess_GenSmoothNormals;

		mScene = mImporter.ReadFile(path.c_str(), flags);

		if (mScene == nullptr)
		{
			LOG_TO_TERMINAL(Logger::Error, "Error: %s when loading model %s", mImporter.GetErrorString(), path.c_str());
			return;
		}

		// loads
		mEntries.resize(mScene->mNumMeshes);

		for (unsigned int i = 0; i < mEntries.size(); i++)
		{
			mEntries[i].vertexBase = mNumVertices;
			mNumVertices += mScene->mMeshes[i]->mNumVertices;
		}

		for (unsigned int i = 0; i < mEntries.size(); i++)
		{
			const aiMesh* pMesh = mScene->mMeshes[i];
			InitMesh(i, pMesh, mScene);
		}
	}

	void MeshLoader::CreateRendererResources(std::vector<MeshLoader::VertexLayout> layouts, float scale)
	{
		std::vector<float> vertexBuffer;

		for (int32_t m = 0; m < mEntries.size(); m++)
		{
			for (int32_t i = 0; i < mEntries[m].vertices.size(); i++)
			{
				for (auto& layout : layouts)
				{
					// todo:: check if vertex component exists
					switch (layout)
					{
						case MeshLoader::VERTEX_LAYOUT_POSITION:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].position.x * scale);
							vertexBuffer.push_back(mEntries[m].vertices[i].position.y * scale);
							vertexBuffer.push_back(mEntries[m].vertices[i].position.z * scale);

							break;
						}
						case MeshLoader::VERTEX_LAYOUT_NORMAL:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].normal.x);
							vertexBuffer.push_back(-mEntries[m].vertices[i].normal.y);
							vertexBuffer.push_back(mEntries[m].vertices[i].normal.z);

							break;
						}
						case MeshLoader::VERTEX_LAYOUT_UV0:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].uv0.s);
							vertexBuffer.push_back(mEntries[m].vertices[i].uv0.t);

							break;
						}
						case MeshLoader::VERTEX_LAYOUT_COLOR:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].color.r);
							vertexBuffer.push_back(mEntries[m].vertices[i].color.g);
							vertexBuffer.push_back(mEntries[m].vertices[i].color.b);

							break;
						}
						case MeshLoader::VERTEX_LAYOUT_TANGENT:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].tangent.x);
							vertexBuffer.push_back(mEntries[m].vertices[i].tangent.y);
							vertexBuffer.push_back(mEntries[m].vertices[i].tangent.z);

							break;
						}
						case MeshLoader::VERTEX_LAYOUT_BITANGENT:
						{
							vertexBuffer.push_back(mEntries[m].vertices[i].bitangent.x);
							vertexBuffer.push_back(mEntries[m].vertices[i].bitangent.y);
							vertexBuffer.push_back(mEntries[m].vertices[i].bitangent.z);

							break;
						}
					}
				}
			}
		}

		size_t vertexBufferSize = vertexBuffer.size() * sizeof(float);
		std::vector<uint32_t> indexBuffer;

		for (uint32_t m = 0; m < mEntries.size(); m++)
		{
			uint32_t indexBase = (uint32_t)indexBuffer.size();
			for (uint32_t i = 0; i < mEntries[m].indices.size(); i++)
			{
				indexBuffer.push_back(mEntries[m].indices[i] + indexBase);
			}
		}

		size_t indexBufferSize = indexBuffer.size() * sizeof(uint32_t);

		// create vertex buffer
		{
			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.pNext = nullptr;
			bufferCI.flags = 0;
			bufferCI.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
			bufferCI.size = vertexBufferSize;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VK_ASSERT(vkCreateBuffer(mRenderer->GetDevice()->GetDevice(), &bufferCI, nullptr, &mVertexBuffer), "Failed to create buffer");

			// alocate memory for specified buffer 
			VkMemoryRequirements memoryReqs = {};
			vkGetBufferMemoryRequirements(mRenderer->GetDevice()->GetDevice(), mVertexBuffer, &memoryReqs);

			VkMemoryAllocateInfo memoryAllocInfo = {};
			memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocInfo.pNext = nullptr;
			memoryAllocInfo.allocationSize = memoryReqs.size;
			memoryAllocInfo.memoryTypeIndex = mRenderer->GetDevice()->GetMemoryType(memoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VK_ASSERT(vkAllocateMemory(mRenderer->GetDevice()->GetDevice(), &memoryAllocInfo, nullptr, &mVertexMemory), "Failed to allocate memory for buffer");

			// map the buffer and copy the data
			void* mapped;
			VK_ASSERT(vkMapMemory(mRenderer->GetDevice()->GetDevice(), mVertexMemory, 0, vertexBufferSize, 0, &mapped), "Faled to map memory");
			memcpy(mapped, vertexBuffer.data(), vertexBufferSize);
			vkUnmapMemory(mRenderer->GetDevice()->GetDevice(), mVertexMemory);

			// link buffer with allocated memory
			VK_ASSERT(vkBindBufferMemory(mRenderer->GetDevice()->GetDevice(), mVertexBuffer, mVertexMemory, 0), "Failed to bind buffer with memory");
		}
		
		// create index buffer
		{
			VkBufferCreateInfo bufferCI = {};
			bufferCI.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
			bufferCI.pNext = nullptr;
			bufferCI.flags = 0;
			bufferCI.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
			bufferCI.size = indexBufferSize;
			bufferCI.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			VK_ASSERT(vkCreateBuffer(mRenderer->GetDevice()->GetDevice(), &bufferCI, nullptr, &mIndexBuffer), "Failed to create buffer");

			// alocate memory for specified buffer 
			VkMemoryRequirements memoryReqs = {};
			vkGetBufferMemoryRequirements(mRenderer->GetDevice()->GetDevice(), mIndexBuffer, &memoryReqs);

			VkMemoryAllocateInfo memoryAllocInfo = {};
			memoryAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
			memoryAllocInfo.pNext = nullptr;
			memoryAllocInfo.allocationSize = memoryReqs.size;
			memoryAllocInfo.memoryTypeIndex = mRenderer->GetDevice()->GetMemoryType(memoryReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
			VK_ASSERT(vkAllocateMemory(mRenderer->GetDevice()->GetDevice(), &memoryAllocInfo, nullptr, &mIndexMemory), "Failed to allocate memory for buffer");

			// map the buffer and copy the data
			void* mapped;
			VK_ASSERT(vkMapMemory(mRenderer->GetDevice()->GetDevice(), mIndexMemory, 0, indexBufferSize, 0, &mapped), "Faled to map memory");
			memcpy(mapped, indexBuffer.data(), indexBufferSize);
			vkUnmapMemory(mRenderer->GetDevice()->GetDevice(), mIndexMemory);
			
			// link buffer with allocated memory
			VK_ASSERT(vkBindBufferMemory(mRenderer->GetDevice()->GetDevice(), mIndexBuffer, mIndexMemory, 0), "Failed to bind buffer with memory");
			
			mIndexCount = (uint32_t)indexBuffer.size();
		}
	}

	void MeshLoader::DestroyResources()
	{
		vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mVertexBuffer, nullptr);
		vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mVertexMemory, nullptr);
		vkDestroyBuffer(mRenderer->GetDevice()->GetDevice(), mIndexBuffer, nullptr);
		vkFreeMemory(mRenderer->GetDevice()->GetDevice(), mIndexMemory, nullptr);
	}

	void MeshLoader::InitMesh(unsigned int index, const aiMesh* mesh, const aiScene* scene)
	{
		mEntries[index].materialIndex = mesh->mMaterialIndex;

		aiColor3D pColor(0.f, 0.f, 0.f);
		scene->mMaterials[mesh->mMaterialIndex]->Get(AI_MATKEY_COLOR_DIFFUSE, pColor);

		aiVector3D Zero3D(0.0f, 0.0f, 0.0f);

		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			aiVector3D* pPos = &(mesh->mVertices[i]);
			aiVector3D* pNormal = &(mesh->mNormals[i]);
			aiVector3D* pTexCoord;

			if (mesh->HasTextureCoords(0)) pTexCoord = &(mesh->mTextureCoords[0][i]);
			else pTexCoord = &Zero3D;

			aiVector3D* pTangent = (mesh->HasTangentsAndBitangents()) ? &(mesh->mTangents[i]) : &Zero3D;
			aiVector3D* pBiTangent = (mesh->HasTangentsAndBitangents()) ? &(mesh->mBitangents[i]) : &Zero3D;

			Vertex v;
			v.position = glm::vec3(pPos->x, pPos->y, pPos->z);
			v.uv0 = glm::vec2(pTexCoord->x, pTexCoord->y);
			v.normal = glm::vec3(pNormal->x, pNormal->y, pNormal->z);
			v.color = glm::vec3(pColor.r, pColor.g, pColor.b);
			v.tangent = glm::vec3(pTangent->x, pTangent->y, pTangent->z);
			v.bitangent = glm::vec3(pColor.r, pColor.g, pColor.b);

			mDimension.max.x = fmax(pPos->x, mDimension.max.x);
			mDimension.max.y = fmax(pPos->y, mDimension.max.y);
			mDimension.max.z = fmax(pPos->z, mDimension.max.z);

			mDimension.min.x = fmin(pPos->x, mDimension.min.x);
			mDimension.min.y = fmin(pPos->y, mDimension.min.y);
			mDimension.min.z = fmin(pPos->z, mDimension.min.z);

			mEntries[index].vertices.push_back(v);
		}

		mDimension.size = mDimension.max - mDimension.min;

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			const aiFace& face = mesh->mFaces[i];
			assert(face.mNumIndices == 3);
			
			mEntries[index].indices.push_back(face.mIndices[0]);
			mEntries[index].indices.push_back(face.mIndices[1]);
			mEntries[index].indices.push_back(face.mIndices[2]);
		}
	}
}