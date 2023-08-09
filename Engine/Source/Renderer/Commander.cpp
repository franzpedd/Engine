#include "Commander.h"

#include "Vulkan/VKDevice.h"
#include "Util/Logger.h"

namespace Cosmos
{
	std::shared_ptr<CommandEntry> CommandEntry::Create(std::shared_ptr<VKDevice>& device)
	{
		return std::make_shared<CommandEntry>(device);
	}

	CommandEntry::CommandEntry(std::shared_ptr<VKDevice>& device)
		: device(device)
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
			vkDestroyCommandPool(device->Device(), commandPool, nullptr);
			vkFreeCommandBuffers(device->Device(), commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());
		}

		for (size_t i = 0; i < framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device->Device(), framebuffers[i], nullptr);
		}
	}

	std::shared_ptr<Commander> Commander::Create()
	{
		return std::make_shared<Commander>();
	}

	Commander::Commander()
	{
		Logger() << "Creating Commander";
	}

	Commander::~Commander()
	{
	}

	void Commander::Add(CommandEntry* entry)
	{
		mEntrylist.emplace_back(entry);
	}

	void Commander::Pop(CommandEntry* entry)
	{
		auto it = std::find(mEntrylist.begin(), mEntrylist.end(), entry);
		if (it != mEntrylist.end())
		{
			mEntrylist.erase(it);
		}
	}
}