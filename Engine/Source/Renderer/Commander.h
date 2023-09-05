#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <string>
#include <vector>

namespace Cosmos
{

	struct CommandEntry
	{
	public:

		// returns a smart ptr of a new commander entry
		static std::shared_ptr<CommandEntry> Create(VkDevice& device, std::string name);

		// constructor
		CommandEntry(VkDevice& device, std::string name);

		// destructor
		~CommandEntry() = default;

		// free all used resources
		void Destroy();

		// returns the name of the command entry
		inline std::string& Name() { return name; }

	public:

		VkDevice& device;
		std::string name;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> commandBuffers = {};
		std::vector<VkFramebuffer> framebuffers = {};
	};

	class Commander
	{
	public:

		// constructor
		Commander();

		// destructor
		~Commander();

		// returns a reference to the commander singleton
		static Commander& Get() { return *sCommander; }

		// access the command entries 
		std::vector<std::shared_ptr<CommandEntry>>& Access() { return mEntrylist; }

	public:

		// adds a new commander entry from the list
		void Add(std::shared_ptr<CommandEntry> entry);

		// removes a commander entry from the list
		void Pop(std::shared_ptr<CommandEntry> entry);

		// outputs the name of registered commanders
		void Print();

	private:

		static Commander* sCommander;
		std::vector<std::shared_ptr<CommandEntry>> mEntrylist; // todo: move to unordered map
	};
}