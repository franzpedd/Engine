#pragma once

#include <vulkan/vulkan.h>
#include <memory>
#include <vector>

namespace Cosmos
{
	// forward declaration
	class VKDevice;

	struct CommandEntry
	{
		std::shared_ptr<VKDevice>& device;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkRenderPass renderPass = VK_NULL_HANDLE;
		VkCommandPool commandPool = VK_NULL_HANDLE;
		std::vector<VkCommandBuffer> commandBuffers = {};
		std::vector<VkFramebuffer> framebuffers = {};

		// returns a smart ptr of a new commander entry
		static std::shared_ptr<CommandEntry> Create(std::shared_ptr<VKDevice>& device);

		// constructor
		CommandEntry(std::shared_ptr<VKDevice>& device);

		// destructor
		~CommandEntry();
	};

	class Commander
	{

	public:

		// returns a reference to a new renderer commander
		static std::shared_ptr<Commander> Create();

		// constructor
		Commander();

		// destructor
		~Commander();

	public:

		// adds a new commander entry from the list
		void Add(CommandEntry* entry);

		// removes a commander entry from the list
		void Pop(CommandEntry* entry);

	private:

		std::vector<CommandEntry*> mEntrylist;

	};
}