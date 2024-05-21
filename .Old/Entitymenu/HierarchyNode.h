#pragma once

#include <Engine.h>

namespace Cosmos
{
    class HierarchyBase
    {
    public:

        // constructor
        HierarchyBase() = default;

        // destructor
        ~HierarchyBase() = default;
    
    public:

        // selection-context of this node
        bool selected = false;

        // pointer to the entity
        Entity* entity = nullptr;
    };

    class HierarchyGroup
    {
    public:

        // constructor
        HierarchyGroup() = default;

        // destructor
        ~HierarchyGroup() = default;

    public:
        
        // group name
        std::string name = "Group";

        // the map of entities of this group
        std::multimap<std::string, Shared<HierarchyBase>> entities = {};
    };
}