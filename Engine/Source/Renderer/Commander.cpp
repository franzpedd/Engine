#include "Commander.h"

#include "Vulkan/VKDevice.h"
#include "Util/Logger.h"

namespace Cosmos
{
	Commander* Commander::sCommander = nullptr;

	std::shared_ptr<CommandEntry> CommandEntry::Create(std::shared_ptr<VKDevice>& device, std::string name)
	{
		return std::make_shared<CommandEntry>(device, name);
	}

	CommandEntry::CommandEntry(std::shared_ptr<VKDevice>& device, std::string name)
		: device(device), name(name)
	{
		
	}

	CommandEntry::~CommandEntry()
	{
		vkDeviceWaitIdle(device->Device());

		if (descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(device->Device(), descriptorPool, nullptr);
		}

		if (renderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(device->Device(), renderPass, nullptr);
		}

		if (commandPool != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(device->Device(), commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());
			vkDestroyCommandPool(device->Device(), commandPool, nullptr);
		}

		for (size_t i = 0; i < framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device->Device(), framebuffers[i], nullptr);
		}
	}

	Commander::Commander()
	{
		LOG_ASSERT(sCommander == nullptr, "Commander already exists");
		sCommander = this;

		Logger() << "Creating Commander";
	}

	Commander::~Commander()
	{
		sCommander = nullptr;
	}

	void Commander::Add(std::shared_ptr<CommandEntry> entry)
	{
		mEntrylist.emplace_back(entry);
	}

	void Commander::Pop(std::shared_ptr<CommandEntry> entry)
	{
		auto it = std::find(mEntrylist.begin(), mEntrylist.end(), entry);
		if (it != mEntrylist.end())
		{
			mEntrylist.erase(it);
		}
	}

	void Commander::Print()
	{
		printf("Showing registered commands:\n");
		for (size_t i = 0; i < mEntrylist.size(); i++)
		{
			printf("Reg %d: %s\n", (int32_t)i, mEntrylist[i]->Name().c_str());
		}
	}
}