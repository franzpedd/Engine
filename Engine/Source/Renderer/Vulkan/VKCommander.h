#pragma once

#include "VKDevice.h"
#include "Util/Memory.h"

#include <unordered_map>

namespace Cosmos
{
    // this struct holds all commands related to a specific render pass object
    struct VKCommandEntry
    {
        std::string nameid = {};
        VkDevice device = VK_NULL_HANDLE;
        VkCommandPool commandPool = VK_NULL_HANDLE;
        VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
        VkRenderPass renderPass = VK_NULL_HANDLE;
        std::vector<VkCommandBuffer> commandBuffers = {};
        std::vector<VkFramebuffer> frameBuffers = {};
        VkSampleCountFlagBits msaa = VK_SAMPLE_COUNT_1_BIT;

        // constructor
        VKCommandEntry(std::string name, VkDevice device);

        // destructor
        ~VKCommandEntry();
    };

    // holds all command entries
    class VKCommander
    {
    public:

        // constructor
        VKCommander();

        // destructor
        ~VKCommander();

        // returns the vkcommander singleton
        inline static VKCommander* GetInstance() { return sCommander; }

        // returns a reference to all entries
        inline std::unordered_map<std::string, Shared<VKCommandEntry>>& GetEntriesRef() { return mEntries; };

        // returns a reference to the primary/main VKCommandEntry
        inline Shared<VKCommandEntry>& GetMainRef() { return mMainEntry; }

    public:

        // sets a new main VKCommandEntry by its nameid, does nothing if entry doesn't exists and returns false
        bool SetMain(std::string nameid);

        // returns if VKCommandEntry exists
        bool Exists(std::string nameid);

        // inserts a new VKCommandEntry, returns false if already exists
        bool Insert(std::string nameid, VkDevice device);

        // erases a given VKCommandEntry given it's nameid, returns false if doesn't exists
        bool Erase(std::string nameid);

    private:

        static VKCommander* sCommander;
        Shared<VKCommandEntry> mMainEntry;
        std::unordered_map<std::string, Shared<VKCommandEntry>> mEntries;
    };
}