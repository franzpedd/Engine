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
        ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_MenuBar);

        // hierarchy top menu
        if(ImGui::BeginMenuBar())
        {
            ImGui::BeginGroup();
			{
                ImGui::Text(ICON_FA_PAINT_BRUSH " Edit Entity");

				float itemSize = 35.0f;
                float itemCount = 2.0f;

				ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (itemSize * itemCount--));

				if (ImGui::MenuItem(ICON_FA_PLUS_SQUARE))
				{
                    Shared<HierarchySingle> node = CreateShared<HierarchySingle>();
                    node->type = HierarchyType::Single;

                    mHierarchyNodes.push_back(node);
				}

                ImGui::SetCursorPosX(ImGui::GetWindowWidth() - (itemSize * itemCount--));

                if (ImGui::MenuItem(ICON_FA_FOLDER))
				{
                    Shared<HierarchyGroup> node = CreateShared<HierarchyGroup>();
                    node->type = HierarchyType::Group;

                    mHierarchyNodes.push_back(node);
				}
			}
			ImGui::EndGroup();

            ImGui::EndMenuBar();
        }

        // draw nodes
        uint32_t selectedNodesCount = GetSelectedNodesCount();
        
        for(auto& node : mHierarchyNodes)
        {
            // push id for uniquely identify this node
            ImGui::PushID((void*)node.get());

            switch(node->type)
            {
                case HierarchyType::Single:
                {
                    ImGui::Text(ICON_LC_BOX);
                    ImGui::SameLine();

                    if (ImGui::Selectable(
                        "Single Entity", 
                        &node->selected, 
                        ImGuiSelectableFlags_DontClosePopups))
			        {
                        if(selectedNodesCount > 0 && !ImGui::GetIO().KeyCtrl)
                            node->selected = false;
			        }
                    
                    break;
                }

                case HierarchyType::Group:
                {
                    ImGui::Text(ICON_LC_BOXES);
                    ImGui::SameLine();

                    if (ImGui::Selectable(
                        "Group Entity", 
                        &node->selected, 
                        ImGuiSelectableFlags_DontClosePopups))
			        {
                        if(selectedNodesCount > 0 && !ImGui::GetIO().KeyCtrl)
                            node->selected = false;
			        }

                    break;
                }
            }

            ImGui::PopID();
        }

        ImGui::End();
    }

    uint32_t Hierarchy::GetSelectedNodesCount()
    {
        uint32_t count = 0;

        for(auto& node : mHierarchyNodes)
        {
            if(node->selected)
                count++;
        }

        return count;
    }
}