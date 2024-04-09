#include "SceneGraph.h"
#include <imgui/imgui_internal.h>

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

        DrawComponents();

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

	static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
	{
		ImGuiIO& io = ImGui::GetIO();
		auto boldFont = io.Fonts->Fonts[0];

		ImGui::PushID(label.c_str());

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();

		ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
		ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("X", buttonSize))
			values.x = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Y", buttonSize))
			values.y = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();
		ImGui::SameLine();

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
		ImGui::PushFont(boldFont);
		if (ImGui::Button("Z", buttonSize))
			values.z = resetValue;
		ImGui::PopFont();
		ImGui::PopStyleColor(3);

		ImGui::SameLine();
		ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
		ImGui::PopItemWidth();

		ImGui::PopStyleVar();

		ImGui::Columns(1);

		ImGui::PopID();
	}

	template<typename T, typename UIFunction>
	static void DrawComponent(const std::string& name, Entity* entity, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;
		if (entity->HasComponent<T>())
		{
			auto component = entity->GetComponent<T>();
			ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
			float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;
			ImGui::Separator();
			bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, name.c_str());
			ImGui::PopStyleVar(
			);
			ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
			if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
			{
				ImGui::OpenPopup("ComponentSettings");
			}

			bool removeComponent = false;
			if (ImGui::BeginPopup("ComponentSettings"))
			{
				if (ImGui::MenuItem("Remove component"))
					removeComponent = true;

				ImGui::EndPopup();
			}

			if (open)
			{
				uiFunction(component);
				ImGui::TreePop();
			}

			if (removeComponent)
				entity->RemoveComponent<T>();
		}
	}

    void SceneGraph::DrawComponents()
    {
		auto entity = m_Scene->GetEntity(m_SelectionContext);
		if (entity == nullptr) {
			return;
		}

		if (entity->HasComponent<TagComponent>())
		{
			auto tag = entity->GetComponent<TagComponent>();

			char buffer[256];
			memset(buffer, 0, sizeof(buffer));
			strncpy_s(buffer, sizeof(buffer), tag->name.c_str(), sizeof(buffer));
			if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
			{
				tag->name = std::string(buffer);
			}
		}

		ImGui::SameLine();
		ImGui::PushItemWidth(-1);

		if (ImGui::Button("Add Component"))
			ImGui::OpenPopup("AddComponent");

		if (ImGui::BeginPopup("AddComponent"))
		{
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<MaterialComponent>("Material");
			DisplayAddComponentEntry<MeshComponent>("Mesh");
			DisplayAddComponentEntry<PointLightComponent>("Point Light");

			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component)
		{
			DrawVec3Control("Translation", component->local_transform.translation);
			glm::vec3 rotation = glm::degrees(component->local_transform.rotation);
			DrawVec3Control("Rotation", rotation);
			component->local_transform.rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component->local_transform.scale, 1.0f);
		});

		DrawComponent<CameraComponent>("Camera", entity, [](auto& component)
		{
			auto& camera = component->camera;

			ImGui::Checkbox("Primary", &component->primary);

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = camera->GetType() == CameraType::PERSPECTIVE ? "Perspective" : "Orthographic";
			if (ImGui::BeginCombo("Projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera->ChangeCameraType(i == 0 ? CameraType::PERSPECTIVE : CameraType::ORTHOGRAPHIC);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			float perspectiveVerticalFov = glm::degrees(camera->GetFOVY());
			if (ImGui::DragFloat("Vertical FOV", &perspectiveVerticalFov))
				camera->SetFovy(glm::radians(perspectiveVerticalFov));

			float perspectiveNear = camera->GetNear();
			if (ImGui::DragFloat("Near", &perspectiveNear))
				camera->SetNear(perspectiveNear);

			float perspectiveFar = camera->GetFar();
			if (ImGui::DragFloat("Far", &perspectiveFar))
				camera->SetFar(perspectiveFar);
		});


		DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component)
		{
			auto& light = component->light;
			float color[3] = { light->GetColor().r,  light->GetColor().g, light->GetColor().b };
			ImGui::Text("Light color:");
			ImGui::ColorEdit3("##light_color", color);
			light->SetColor(glm::vec3(color[0], color[1], color[2]));

			float intensity = light->GetIntensity();
			ImGui::Text("Intensity:");
			ImGui::DragFloat("##light_intensity", &intensity, 0.1f);
			light->SetIntensity(intensity);

			float attenuations[3] = { light->GetConstantAttenuation(),  light->GetLinearAttenuation(), light->GetQuadraticAttenuation() };
			ImGui::Text("Attenuations (constant, linear, quadratic):");
			ImGui::DragFloat3("##light_attenuation", attenuations, 0.1f);
			light->SetConstantAttenuation(attenuations[0]);
			light->SetLinearAttenuation(attenuations[1]);
			light->SetQuadraticAttenuation(attenuations[2]);
		});

		DrawComponent<MeshComponent>("Mesh", entity, [](auto& component)
		{
			ImGui::Text("Nothing yet :(");
		});

		DrawComponent<MaterialComponent>("Material", entity, [](auto& component)
		{
			ImGui::Text("Nothing yet :(");
		});
    }


	template<typename T>
	void SceneGraph::DisplayAddComponentEntry(const std::string& entryName) {
		auto entity = m_Scene->GetEntity(m_SelectionContext);
		if (!entity->HasComponent<T>())
		{
			if (ImGui::MenuItem(entryName.c_str()))
			{
				entity->AddComponent<T>(T());
				ImGui::CloseCurrentPopup();
			}
		}
	}

}