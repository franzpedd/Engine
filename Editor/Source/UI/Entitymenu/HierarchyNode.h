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
    };

    class HierarchySingle : public HierarchyBase
    {
    public:

        // constructor
        HierarchySingle() = default;

        // destructor
        ~HierarchySingle() = default;

    public:

        // the entity of this node
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
        std::vector<Entity*> entities = {};
    };
}