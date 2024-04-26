#include "epch.h"
#include "VKCommander.h"

namespace Cosmos
{
    VKCommandEntry::VKCommandEntry(std::string name, VkDevice device)
        : nameid(name), device(device)
    {
        LOG_TO_TERMINAL(Logger::Severity::Trace, "Creating VKCommandEntry %s", nameid.c_str());
    }

    VKCommandEntry::~VKCommandEntry()
    {
        vkDeviceWaitIdle(device);

        vkDestroyDescriptorPool(device, descriptorPool, nullptr);
        vkDestroyRenderPass(device, renderPass, nullptr);

        vkFreeCommandBuffers(device, commandPool, (uint32_t)commandBuffers.size(), commandBuffers.data());
        vkDestroyCommandPool(device, commandPool, nullptr);

        for (auto& framebuffer : frameBuffers)
            vkDestroyFramebuffer(device, framebuffer, nullptr);

        frameBuffers.clear();
    }

    VKCommander* VKCommander::sCommander = nullptr;

    VKCommander::VKCommander()
    {
        LOG_TO_TERMINAL(Logger::Severity::Trace, "Creating Vulkan Commander");
        sCommander = this;
    }

    VKCommander::~VKCommander()
    {
        sCommander = nullptr;

        mEntries.clear();
    }

    bool VKCommander::SetMain(std::string nameid)
    {
        auto it = mEntries.find(nameid);

        if (it != mEntries.end())
        {
            mMainEntry = mEntries[nameid];
            return true;
        }

        return false;
    }

    bool VKCommander::Exists(std::string nameid)
    {
        return mEntries.find(nameid) != mEntries.end() ? true : false;
    }

    bool VKCommander::Insert(std::string nameid, VkDevice device)
    {
        auto it = mEntries.find(nameid);

        if (it != mEntries.end())
        {
            LOG_TO_TERMINAL(Logger::Severity::Error, "An entry with name %s already exists", nameid.c_str());
            return false;
        }

        mEntries[nameid] = CreateShared<VKCommandEntry>(nameid, device);
        return true;
    }

    bool VKCommander::Erase(std::string nameid)
    {
        auto it = mEntries.find(nameid);

        if (it != mEntries.end())
        {
            mEntries.erase(nameid);
            return true;
        }

        LOG_TO_TERMINAL(Logger::Severity::Error, "No entry %s exists in the commander", nameid.c_str());
        return false;
    }
}