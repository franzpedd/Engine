#include "epch.h"
#include "Commander.h"

#include "Device.h"

namespace Cosmos
{
	Commander* Commander::sCommander = nullptr;

	std::shared_ptr<Commander::Entry> Commander::Entry::Create(std::string name)
	{
		return std::make_shared<Commander::Entry>(name);
	}

	Commander::Entry::Entry(std::string name)
		: name(name)
	{
	}

	Commander::Entry::~Entry()
	{
		if(descriptorPool != VK_NULL_HANDLE || renderPass != VK_NULL_HANDLE || commandPool != VK_NULL_HANDLE || framebuffers.size() != 0)
		{
			Logger() << "A command entry was not properly destroyed and will result in errors";
		}
	}

	void Commander::Entry::FreeResources(VkDevice device)
	{
		vkDeviceWaitIdle(device);

		if (descriptorPool != VK_NULL_HANDLE)
		{
			vkDestroyDescriptorPool(device, descriptorPool, nullptr);
			descriptorPool = VK_NULL_HANDLE;
		}

		if (renderPass != VK_NULL_HANDLE)
		{
			vkDestroyRenderPass(device, renderPass, nullptr);
			renderPass = VK_NULL_HANDLE;
		}

		if (commandPool != VK_NULL_HANDLE)
		{
			vkFreeCommandBuffers(device, commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());
			vkDestroyCommandPool(device, commandPool, nullptr);
			commandPool = VK_NULL_HANDLE;
		}

		for (size_t i = 0; i < framebuffers.size(); i++)
		{
			vkDestroyFramebuffer(device, framebuffers[i], nullptr);
			framebuffers[i] = VK_NULL_HANDLE;
		}

		framebuffers.clear();
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

	Commander& Commander::Get()
	{
		return *sCommander;
	}

	std::unordered_map<std::string, std::shared_ptr<Commander::Entry>>& Commander::GetEntries()
	{
		return mEntries;
	}

	bool Commander::Exists(std::string id)
	{
		return mEntries.find(id) != mEntries.end() ? true : false;
	}

	bool Commander::MakePrimary(std::string id)
	{
		auto it = mEntries.find(id);

		if (it != mEntries.end())
		{
			mPrimaryEntry = mEntries[id];
			return true;
		}

		return false;
	}

	std::shared_ptr<Commander::Entry>& Commander::GetPrimary()
	{
		return mPrimaryEntry;
	}

	bool Commander::Insert(std::string id)
	{
		auto it = mEntries.find(id);

		if (it != mEntries.end())
		{
			LOG_TO_TERMINAL(Logger::Error, "An entry with name %s already exists", id.c_str());
			return false;
		}

		mEntries[id] = Entry::Create(id);
		return true;
	}

	bool Commander::Erase(std::string id, VkDevice device)
	{
		auto it = mEntries.find(id);

		if (it != mEntries.end())
		{
			mEntries[id]->FreeResources(device);
			mEntries.erase(id);
			return true;
		}

		LOG_TO_TERMINAL(Logger::Error, "No entry %s exists in the commander", id.c_str());
		return false;
	}
}