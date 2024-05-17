#pragma once

#include "HierarchyNode.h"

#include <Engine.h>
#include <map>
#include <vector>

namespace Cosmos
{
    class Hierarchy
    {
    public:

        struct PayloadHelper
        {
            Shared<HierarchyGroup> group;
            size_t index;
        };

    public:

        // constructor
        Hierarchy();

        // destructor
        ~Hierarchy();

    public:

        // tick-update
        void OnUpdate();

    private:

        // draw groups of entities
        void DrawGroups();

        // draw a single entity
        void DrawEntityNode(Shared<HierarchyGroup> group, Shared<HierarchyBase> base, bool* redraw);

    private:

        std::multimap<std::string, Shared<HierarchyGroup>> mGroups;
        size_t mEntitiesID = 0;
        
        PayloadHelper mPayloadHelper = {};
    };
}