#include "Hierarchy.h"

#include <Engine.h>

namespace Cosmos
{
    Hierarchy::Hierarchy()
    {

    }

    Hierarchy::~Hierarchy()
    {

    }

    void Hierarchy::OnUpdate()
    {
        ImGui::Begin("Entities", nullptr);

        // right-click menu
        if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight))
        {
            if (ImGui::MenuItem("Add Group"))
            {
                std::string id = "Group ";
                id.append(std::to_string(mGroupsID++));

                Shared<HierarchyGroup> node = CreateShared<HierarchyGroup>();
                node->type = HierarchyType::Group;
                node->name = id;
                
                mGroups.insert({ id, node });
            }

            ImGui::EndPopup();
        }

        DrawFromRoot();

        ImGui::End();
    }

    void Hierarchy::DrawFromRoot()
    {
        size_t itCount = 0;

        // draw groups
        // top drop behaviour
        DropBehaviour(itCount);

        for (auto& group : mGroups)
        {
            // above drag and drop behavior
            DragBehaviour(itCount);

            // retrieve node's name
            auto& groupName = group.second->name;
            char groupBuffer[ENTITY_NAME_MAX_CHARS];
            memset(groupBuffer, 0, sizeof(groupBuffer));
            std::strncpy(groupBuffer, groupName.c_str(), sizeof(groupBuffer));

            ImGuiTreeNodeFlags flags = {};
            if (ImGui::TreeNodeEx(group.second.get(), flags, groupName.c_str()))
            {
                // group right-menu
                if (ImGui::BeginPopupContextItem(nullptr, ImGuiPopupFlags_MouseButtonRight))
                {
                    if (ImGui::MenuItem("Add Entity"))
                    {
                        Shared<HierarchyBase> base = CreateShared<HierarchyBase>();
                        base->entity = Application::GetInstance()->GetActiveScene()->CreateEntity();
                        group.second->entities.push_back(base);
                    }

                    ImGui::EndPopup();
                }

                // draw children nodes
                for (size_t i = 0; i < group.second->entities.size(); i++)
                {
                    //auto& nodeName = group.second->entities[i]->GetComponent<NameComponent>().name;
                    char nodeBuffer[ENTITY_NAME_MAX_CHARS];
                    memset(nodeBuffer, 0, sizeof(nodeBuffer));
                    std::strncpy(nodeBuffer, nodeName.c_str(), sizeof(nodeBuffer));

                    ImGuiSelectableFlags flags = {};
                    flags |= ImGuiSelectableFlags_DontClosePopups;
                    if (Cosmos::SelectableInputText(nodeName.c_str(), &group.second->, flags, nodeBuffer, sizeof(nodeBuffer)))
                    {
                        nodeName = std::string(nodeBuffer);
                    }
                }

                ImGui::TreePop();
            }

            // under drop behaviour
            DropBehaviour(itCount);

            // increment counter
            itCount++;
        }

        // bottom drop behaviour
        DragBehaviour(itCount);
    }

    void Hierarchy::DragBehaviour(size_t from)
    {
        ImGuiDragDropFlags src_flags = 0;
        src_flags |= ImGuiDragDropFlags_SourceAllowNullID;
        src_flags |= ImGuiDragDropFlags_SourceNoDisableHover;
        src_flags |= ImGuiDragDropFlags_SourceNoHoldToOpenOthers;

        if (ImGui::BeginDragDropSource(src_flags))
        {
            ImGui::SetDragDropPayload("HIERARCHYNODE_PAYLOAD", &from, sizeof(size_t));
            ImGui::EndDragDropSource();
        }
    }

    void Hierarchy::DropBehaviour(size_t to)
    {
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("HIERARCHYNODE_PAYLOAD"))
            {
                mPayloadHelper.from = *(const size_t*)payload->Data;
                mPayloadHelper.to = to;

                // moving stuff around
                //if (mPayloadHelper.from != mPayloadHelper.to)
                //{
                //    if (mHierarchyNodes[mPayloadHelper.to]->type == HierarchyType::Single)
                //    {
                //
                //    }
                //}
            }

            ImGui::EndDragDropTarget();
        }
    }
}