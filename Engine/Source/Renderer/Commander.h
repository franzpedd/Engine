#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class Device;

	class Commander
	{
	public:

		struct Entry
		{
			std::string name = {};
			VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
			VkRenderPass renderPass = VK_NULL_HANDLE;
			VkCommandPool commandPool = VK_NULL_HANDLE;
			std::vector<VkCommandBuffer> commandBuffers = {};
			std::vector<VkFramebuffer> framebuffers = {};
			VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;

		public:

			// creates an commander entry, enforcing the device pointer to not be null
			static std::shared_ptr<Entry> Create(std::string name);

			// constructor
			Entry(std::string name);

			// destructor
			~Entry();

		public:

			// release any pointer that may be in used by this entry
			void FreeResources(VkDevice device);
		};

	public:

		// constructor 
		Commander();

		// destructor
		~Commander();

		// returns the commander singleton
		static Commander& Get();

	public:

		// returns a reference to all entries in the commander, allowing accesss 
		std::unordered_map<std::string, std::shared_ptr<Entry>>& GetEntries();

		// returns if an entry with a given name exists
		bool Exists(std::string id);

		// sets the primary command entry to be used by the renderer, doesnt do anything if id is not found on the entry map
		bool MakePrimary(std::string id);

		// returns a referencee to the primary command entry
		std::shared_ptr<Entry>& GetPrimary();

		// adds a new command entry
		bool Insert(std::string id);

		// erases a command entry
		bool Erase(std::string id, VkDevice device);

	private:

		static Commander* sCommander;
		std::unordered_map<std::string, std::shared_ptr<Entry>> mEntries;
		std::shared_ptr<Entry> mPrimaryEntry = nullptr;
	};
}