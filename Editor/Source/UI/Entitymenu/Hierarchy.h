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
            enum PayloadOperation { NONE = 0, MOVE, MERGE, UNMERGE } operation;
            size_t from;
            size_t to;
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

        // draw entities located on root
        void DrawFromRoot();

        // sets a payload from a location
        void DragBehaviour(size_t from);

        // drops the payload to a location
        void DropBehaviour(size_t to);

    private:

        std::map<std::string, Shared<HierarchyGroup>> mGroups;
        size_t mGroupsID = 0;
        size_t mEntitiesID = 0;
        PayloadHelper mPayloadHelper = {};
    };
}