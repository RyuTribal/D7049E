#include "SceneGraph.h"

namespace EditorPanels {
	SceneGraph* SceneGraph::s_Instance = nullptr;
	void SceneGraph::RenderImpl()
	{
		ImGui::Begin("Scene Hierarchy");

		ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
		node_flags |= ImGuiTreeNodeFlags_DefaultOpen;

		std::string root_node_name = "(" + m_Scene->GetName() + ")";

		if (ImGui::TreeNodeEx((void*)(intptr_t)m_Scene->GetId(), node_flags, "%s", root_node_name.c_str())) {
            if (ImGui::BeginDragDropTarget()) {
                const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_NODE");
                if (payload) {
                    UUID droppedNodeId = *(const UUID*)payload->Data;
                    m_Scene->ReparentSceneNode(&droppedNodeId, &m_Scene->GetId());
                }
                ImGui::EndDragDropTarget();
            }
			for (const auto& child : *m_Scene->GetRootNode()->GetChildren()) {
				ImGui::Dummy(ImVec2(0.0f, 5.0f));
				DisplaySceneEntity(child.get());
			}
			ImGui::TreePop();
		}

		if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered()) {
			m_SelectionContext = {};
		}
			

		if (ImGui::BeginPopupContextWindow(0, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
		{
			if (ImGui::MenuItem("Create Empty Entity"))
				m_Scene->CreateEntity("Empty Entity", nullptr);

			ImGui::EndPopup();
		}

		ImGui::End();

		ImGui::Begin("Properties");

		ImGui::End();

	}
    void SceneGraph::DisplaySceneEntity(SceneNode* node)
    {
        if (node == nullptr) {
            return;
        }

        auto entity = m_Scene->GetEntity(node->GetID());

        if (entity == nullptr) {
            return;
        }

        ImGui::PushID(node->GetID());

        ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow;
        if (node->GetChildren()->empty()) {
            node_flags |= ImGuiTreeNodeFlags_Leaf;
        }

        std::string entity_header = entity->GetComponent<TagComponent>()->name;
        if (!node->GetChildren()->empty()) {
            entity_header += " (" + std::to_string(node->GetChildren()->size()) + ")";
        }

        bool node_open = ImGui::TreeNodeEx("##arrow", node_flags);

        ImGui::SameLine();
        ImGuiSelectableFlags button_flags = ImGuiSelectableFlags_AllowDoubleClick;
        if (node->GetID() == m_SelectionContext) {
            button_flags |= ImGuiTreeNodeFlags_Selected;
        }
        if (ImGui::Selectable(entity_header.c_str(), node->GetID() == m_SelectionContext, button_flags)) {
            m_SelectionContext = node->GetID();
        }

        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
            ImGui::SetDragDropPayload("SCENE_NODE", &node->GetID(), sizeof(UUID));
            ImGui::Text("Move %s", entity_header.c_str());
            ImGui::EndDragDropSource();
        }

        if (ImGui::BeginDragDropTarget()) {
            const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SCENE_NODE");
            if (payload) {
                UUID droppedNodeId = *(const UUID*)payload->Data;
                m_Scene->ReparentSceneNode(&droppedNodeId, &node->GetID());
            }
            ImGui::EndDragDropTarget();
        }

        bool entityDeleted = false;
        if (ImGui::BeginPopupContextItem()) {
            if (ImGui::MenuItem("Create Empty Entity With Current Entity As Parent"))
                m_Scene->CreateEntity("Empty Entity", &node->GetID());
            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;
            ImGui::EndPopup();
        }

        if (node_open) {
            for (auto& child : *node->GetChildren()) {
                DisplaySceneEntity(child.get());
            }
            ImGui::TreePop();
        }

        if (entityDeleted) {
            m_Scene->DestroyEntity(node->GetID());
            if (m_SelectionContext == node->GetID())
                m_SelectionContext = {};
        }

        ImGui::PopID();
    }

}