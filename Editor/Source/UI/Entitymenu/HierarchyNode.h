#pragma once

#include <Engine.h>

namespace Cosmos
{
    enum HierarchyType
    {
        Single = 0,
        Group
    };

    class HierarchyBase
    {
    public:

        // constructor
        HierarchyBase() = default;

        // destructor
        ~HierarchyBase() = default;
    
    public:

        // holds witch type of hierarchy node the entry is
        HierarchyType type = HierarchyType::Single;

        // selection-context of this node
        bool selected = false;

        // the name of this node
        std::string name = {};

        // pointer to the entity
        Entity* entity = nullptr;
    };

    class HierarchyGroup : public HierarchyBase
    {
    public:

        // constructor
        HierarchyGroup() = default;

        // destructor
        ~HierarchyGroup() = default;

    public:

        // the vector of entities of this node
        std::vector<Shared<HierarchyBase>> entities = {};
    };
}