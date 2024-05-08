#pragma once

#include "HierarchyNode.h"

#include <Engine.h>
#include <vector>

namespace Cosmos
{
    class Hierarchy
    {
    public:

        // constructor
        Hierarchy();

        // destructor
        ~Hierarchy();

    public:

        // tick-update
        void OnUpdate();

    public:

        // returns how many entity nodes are selected, kind of inneficient but I'll allow it for now
        uint32_t GetSelectedNodesCount();

    private:

        std::vector<Shared<HierarchyBase>> mHierarchyNodes;
    };
}