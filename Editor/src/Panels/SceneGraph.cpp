#include "SceneGraph.h"
#include <imgui/imgui_internal.h>
#include <imgui/imGuIZMOquat.h>
#include <Sound/AudioAsset.h>

using namespace Engine;

namespace EditorPanels {
	SceneGraph* SceneGraph::s_Instance = nullptr;
	void SceneGraph::RenderImpl()
	{
		if (m_Scene->IsReloading())
		{
			return;
		}
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

	void DrawDirectionControl(const std::string& label, glm::vec3& direction, float columnWidth = 100.0f)
	{

		float azimuth = atan2(direction.z, direction.x);
		float elevation = asin(direction.y / glm::length(direction));

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text((label + " Yaw").c_str());
		ImGui::NextColumn();
		ImGui::SliderAngle("##dir_light_yaw", &azimuth, -180.0f, 180.0f);
		ImGui::Columns(1);

		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, columnWidth);
		ImGui::Text((label + " Pitch").c_str());
		ImGui::NextColumn();
		ImGui::SliderAngle("##dir_light_pitch", &elevation, -90.0f, 90.0f);
		ImGui::Columns(1);

		direction.x = cos(elevation) * cos(azimuth);
		direction.y = sin(elevation);
		direction.z = cos(elevation) * sin(azimuth);
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
			ImGui::PopStyleVar();
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
				uiFunction(component, entity->GetHandle());
				ImGui::TreePop();
			}

			if (removeComponent)
				entity->RemoveComponent<T>();
		}
	}

	void SceneGraph::DrawDropBox(const std::string& label)
	{
		float width = ImGui::GetContentRegionAvail().x;
		ImVec2 boxSize = ImVec2(width, 150);
		ImVec2 cursorPos = ImGui::GetCursorScreenPos();

		ImGui::InvisibleButton(label.c_str(), boxSize);

		bool hovered = ImGui::IsItemHovered();
		ImDrawList* drawList = ImGui::GetWindowDrawList();

		// Define dashed border
		ImVec2 topLeft = cursorPos;
		ImVec2 bottomRight = ImVec2(cursorPos.x + boxSize.x, cursorPos.y + boxSize.y);

		float dashLength = 5.0f;
		ImColor borderColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f); // Gray border

		// Draw top border
		for (float x = topLeft.x; x < bottomRight.x; x += dashLength * 2)
		{
			drawList->AddLine(ImVec2(x, topLeft.y), ImVec2(x + dashLength, topLeft.y), borderColor);
		}

		// Draw bottom border
		for (float x = topLeft.x; x < bottomRight.x; x += dashLength * 2)
		{
			drawList->AddLine(ImVec2(x, bottomRight.y), ImVec2(x + dashLength, bottomRight.y), borderColor);
		}

		// Draw left border
		for (float y = topLeft.y; y < bottomRight.y; y += dashLength * 2)
		{
			drawList->AddLine(ImVec2(topLeft.x, y), ImVec2(topLeft.x, y + dashLength), borderColor);
		}

		// Draw right border
		for (float y = topLeft.y; y < bottomRight.y; y += dashLength * 2)
		{
			drawList->AddLine(ImVec2(bottomRight.x, y), ImVec2(bottomRight.x, y + dashLength), borderColor);
		}

		ImVec2 textSize = ImGui::CalcTextSize(label.c_str());
		ImVec2 textPos = ImVec2(cursorPos.x + (boxSize.x - textSize.x) / 2, cursorPos.y + (boxSize.y - textSize.y) / 2);
		ImGui::GetWindowDrawList()->AddText(textPos, ImColor(1.0f, 1.0f, 1.0f, 1.0f), label.c_str());

	}

    void SceneGraph::DrawComponents()
    {
		auto entity = m_Scene->GetEntity(m_SelectionContext);
		if (entity == nullptr) {
			return;
		}

		std::string id_string = fmt::format("UUID: {}", entity->GetID());

		ImGui::Text(id_string.c_str());

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
			DisplayAddComponentEntry<ScriptComponent>("Script");
			DisplayAddComponentEntry<CameraComponent>("Camera");
			DisplayAddComponentEntry<MeshComponent>("Mesh");
			DisplayAddComponentEntry<PointLightComponent>("Point Light");
			DisplayAddComponentEntry<DirectionalLightComponent>("Directional Light");
			DisplayAddComponentEntry<CharacterControllerComponent>("Character Controller");
			DisplayAddComponentEntry<BoxColliderComponent>("Box Collider");
			DisplayAddComponentEntry<SphereColliderComponent>("Sphere Collider");
			DisplayAddComponentEntry<GlobalSoundsComponent>("Global Sounds");
			DisplayAddComponentEntry<LocalSoundsComponent>("Local Sounds");
			ImGui::EndPopup();
		}

		ImGui::PopItemWidth();

		DrawComponent<TransformComponent>("Transform", entity, [](auto& component, auto entity)
		{
			DrawVec3Control("Translation", component->local_transform.translation);
			glm::vec3 rotation = glm::degrees(component->local_transform.rotation);
			DrawVec3Control("Rotation", rotation);
			component->local_transform.rotation = glm::radians(rotation);
			DrawVec3Control("Scale", component->local_transform.scale, 1.0f);
		});

		DrawComponent<ScriptComponent>("Script", entity, [](auto& component, auto entity) {

			std::string class_name = component->Name.empty() ? "No class selected" : component->Name;
			std::string title = fmt::format("Script class: {}", class_name );

			ImGui::Text(title.c_str());

			if (ImGui::Button("Select class"))
			{
				ImGui::OpenPopup("##ClassSearchPopup");
			}

			if (ImGui::BeginPopup("##ClassSearchPopup"))
			{
				static char buffer[64];
				auto& map = ScriptEngine::GetEntityClasses();
				static std::vector<std::string> filteredResults;
				std::string searchText = buffer;

				if (ImGui::InputText("##ClassPopupSearchBar", buffer, sizeof(buffer)))
				{
					std::transform(searchText.begin(), searchText.end(), searchText.begin(), ::tolower);
					filteredResults.clear();
					for (auto& elements : map)
					{
						if (searchText.empty())
						{
							filteredResults.push_back(elements.first);
							continue;
						}

						std::string lowerClassName = elements.first;
						std::transform(lowerClassName.begin(), lowerClassName.end(), lowerClassName.begin(), ::tolower);

						if (lowerClassName.find(searchText) != std::string::npos)
						{
							filteredResults.push_back(elements.first);
						}
					}
				}

				if (filteredResults.size() < 1 && searchText.empty()) // So it fills the list in the beginning
				{
					for (auto& elements : map)
					{
						filteredResults.push_back(elements.first);
					}
				}

				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
				for (auto& result : filteredResults)
				{
					bool isSelected = (result == buffer);
					if (ImGui::Selectable(result.c_str(), isSelected))
					{
						component->Name = result;
						ImGui::CloseCurrentPopup();
					}
					if (isSelected)
					{
						ImGui::SetItemDefaultFocus();
					}
				}

				ImGui::EndPopup();
			}
			
		});

		static UUID primary_camera_ID = 0;

		DrawComponent<CameraComponent>("Camera", entity, [&](auto& component, auto entity)
		{
			auto& camera = component->camera;

			if (component->IsPrimary && primary_camera_ID == 0)
			{
				primary_camera_ID = entity->GetID();
			}

			bool is_primary = primary_camera_ID == entity->GetID();
			component->IsPrimary = is_primary;
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Primary");
			ImGui::NextColumn();
			if (ImGui::Checkbox("##primary_camera", &is_primary))
			{
				if (is_primary)
				{
					primary_camera_ID = entity->GetID();
				}
				else
				{
					primary_camera_ID = 0;
				}
			}
			ImGui::Columns(1);


			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = camera.GetType() == CameraType::PERSPECTIVE ? "Perspective" : "Orthographic";
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Projection");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##projection", currentProjectionTypeString))
			{
				for (int i = 0; i < 2; i++)
				{
					bool isSelected = currentProjectionTypeString == projectionTypeStrings[i];
					if (ImGui::Selectable(projectionTypeStrings[i], isSelected))
					{
						currentProjectionTypeString = projectionTypeStrings[i];
						camera.ChangeCameraType(i == 0 ? CameraType::PERSPECTIVE : CameraType::ORTHOGRAPHIC);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);
			float perspectiveVerticalFov = camera.GetFOVY();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Vertical FOV");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##fov", &perspectiveVerticalFov))
				camera.SetFovy(perspectiveVerticalFov);

			ImGui::Columns(1);
			float perspectiveNear = camera.GetNear();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Near");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##near", &perspectiveNear))
				camera.SetNear(perspectiveNear);
			ImGui::Columns(1);
			float perspectiveFar = camera.GetFar();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Far");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##far", &perspectiveFar))
				camera.SetFar(perspectiveFar);

			ImGui::Columns(1);

			float camera_zoom = camera.GetZoomDistance();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Zoom");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##zoom", &camera_zoom))
				camera.SetZoomDistance(camera_zoom);
			ImGui::Columns(1);

		});


		DrawComponent<PointLightComponent>("Point Light", entity, [](auto& component, auto entity)
		{
			auto& light = component->light;

			bool casting_shadows = light.IsCastingShadows();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Cast Shadows");
			ImGui::NextColumn();
			if (ImGui::Checkbox("##dir_light_shadow", &casting_shadows))
			{
				light.CastShadows(casting_shadows);
			}
			ImGui::Columns(1);

			float color[3] = { light.GetColor().r,  light.GetColor().g, light.GetColor().b };
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Color");
			ImGui::NextColumn();
			ImGui::ColorEdit3("##point_light_color", color);
			light.SetColor(glm::vec3(color[0], color[1], color[2]));
			ImGui::Columns(1);

			float intensity = light.GetIntensity();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Intensity");
			ImGui::NextColumn();
			ImGui::DragFloat("##point_light_intensity", &intensity, 0.1f, 0.0f, 100.f);
			light.SetIntensity(intensity);
			ImGui::Columns(1);

			float attenuations[3] = { light.GetConstantAttenuation(),  light.GetLinearAttenuation(), light.GetQuadraticAttenuation() };
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Attenuations (constant, linear, quadratic)");
			ImGui::NextColumn();
			ImGui::DragFloat3("##point_light_attenuation", attenuations, 0.1f, 0.0f, 100.f);
			light.SetConstantAttenuation(attenuations[0]);
			light.SetLinearAttenuation(attenuations[1]);
			light.SetQuadraticAttenuation(attenuations[2]);
			ImGui::Columns(1);
		});
		DrawComponent<DirectionalLightComponent>("Directional Light", entity, [](auto& component, auto entity)
		{
			auto& light = component->light;

			bool casting_shadows = light.IsCastingShadows();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Cast Shadows");
			ImGui::NextColumn();
			if (ImGui::Checkbox("##dir_light_shadow", &casting_shadows))
			{
				light.CastShadows(casting_shadows);
			}
			ImGui::Columns(1);

			float color[3] = { light.GetColor().r,  light.GetColor().g, light.GetColor().b };
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Color");
			ImGui::NextColumn();
			ImGui::ColorEdit3("##dir_light_color", color);
			glm::vec3 new_color = glm::vec3(color[0], color[1], color[2]);
			light.SetColor(new_color);
			ImGui::Columns(1);

			float intensity = light.GetIntensity();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Intensity");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##dir_light_intensity", &intensity, 0.1f, 0.0f, 100.f))
			{
				light.SetIntensity(intensity);
			}
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Direction");
			ImGui::NextColumn();
			glm::vec3 curr_direction = light.GetDirection();
			if (ImGui::gizmo3D("##gizmo_light_dir", curr_direction, 100, imguiGizmo::modeDirection))
			{
				light.SetDirection(curr_direction);
			}
			ImGui::Columns(1);

		});

		DrawComponent<CharacterControllerComponent>("Character Controller", entity, [](auto& component, auto entity) {
			//float float_max = FLT_MAX/2.f;

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Mass");
			ImGui::NextColumn();
			float mass = component->Mass;
			ImGui::DragFloat("##character_controller_mass", &mass, 1.0f, 0.0f, FLT_MAX);
			component->Mass = mass;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Halfheight");
			ImGui::NextColumn();
			float halfheight = component->HalfHeight;
			ImGui::DragFloat("##character_controller_halfheight", &halfheight, 1.0f, 0.0f, FLT_MAX);
			component->HalfHeight = halfheight;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Radius");
			ImGui::NextColumn();
			float radius = component->Radius;
			ImGui::DragFloat("##character_controller_radius", &radius, 1.0f, 0.0f, FLT_MAX);
			component->Radius = radius;
			ImGui::Columns(1);

			DrawVec3Control("Offset", component->Offset);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Friction");
			ImGui::NextColumn();
			float friction = component->Friction;
			ImGui::DragFloat("##character_controller_friction", &friction, 0.01f, 0.0f, 1.0f);
			component->Friction = friction;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Restitution");
			ImGui::NextColumn();
			float restitution = component->Restitution;
			ImGui::DragFloat("##character_controller_restitution", &restitution, 0.01f, 0.0f, 1.0f);
			component->Restitution = restitution;
			ImGui::Columns(1);

		});

		DrawComponent<BoxColliderComponent>("Box Collider", entity, [](auto& component, auto entity)
		{

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Mass");
			ImGui::NextColumn();
			float mass = component->Mass;
			ImGui::DragFloat("##Physics_Box_Collider_mass", &mass, 1.0f, 0.0f, FLT_MAX);
			component->Mass = mass;
			ImGui::Columns(1);

			DrawVec3Control("Half Size", component->HalfSize);
			DrawVec3Control("Offset", component->Offset);


			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			std::string current_item = FromMotionTypeToString(component->MotionType);
			ImGui::Text("Motion Type");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##Physics_Box_Collider_type", current_item.c_str()))
			{
				for (auto& type : HEMotionTypes)
				{
					std::string selectable_type = FromMotionTypeToString(type);
					bool is_selected = (current_item == selectable_type.c_str());
					if (ImGui::Selectable(selectable_type.c_str(), is_selected))
					{
						component->MotionType = type;
					}
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);


			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Friction");
			ImGui::NextColumn();
			float friction = component->Friction;
			ImGui::DragFloat("##Physics_Box_Collider_friction", &friction, 0.01f, 0.0f, 1.0f);
			component->Friction = friction;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Restitution");
			ImGui::NextColumn();
			float restitution = component->Restitution;
			ImGui::DragFloat("##Physics_Box_Collider_restitution", &restitution, 0.01f, 0.0f, 1.0f);
			component->Restitution = restitution;
			ImGui::Columns(1);

		});

		DrawComponent<SphereColliderComponent>("Sphere Collider", entity, [](auto& component, auto entity)
		{

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Mass");
			ImGui::NextColumn();
			float mass = component->Mass;
			ImGui::DragFloat("##Physics_Sphere_Collider_mass", &mass, 1.0f, 0.0f, FLT_MAX);
			component->Mass = mass;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Radius");
			ImGui::NextColumn();
			float radius = component->Radius;
			//aImGui::SliderFloat("##Physics_Sphere_Colliderr_radius", &radius, 0.0f, 100.0f);
			ImGui::DragFloat("##Physics_Sphere_Colliderr_radius", &radius, 1.f, 0.0f, FLT_MAX);
			component->Radius = radius;
			ImGui::Columns(1);

			DrawVec3Control("Offset", component->Offset);


			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			std::string current_item = FromMotionTypeToString(component->MotionType);
			ImGui::Text("Motion Type");
			ImGui::NextColumn();
			if (ImGui::BeginCombo("##Physics_Sphere_Collider_type", current_item.c_str()))
			{
				for (auto& type : HEMotionTypes)
				{
					std::string selectable_type = FromMotionTypeToString(type);
					bool is_selected = (current_item == selectable_type.c_str());
					if (ImGui::Selectable(selectable_type.c_str(), is_selected))
					{
						component->MotionType = type;
					}
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Friction");
			ImGui::NextColumn();
			float friction = component->Friction;
			ImGui::DragFloat("##Physics_Sphere_Collider_friction", &friction, 0.01f, 0.0f, 1.0f);
			component->Friction = friction;
			ImGui::Columns(1);

			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Restitution");
			ImGui::NextColumn();
			float restitution = component->Restitution;
			ImGui::DragFloat("##Physics_Sphere_Collider_restitution", &restitution, 0.01f, 0.0f, 1.0f);
			component->Restitution = restitution;
			ImGui::Columns(1);

		});

		DrawComponent<GlobalSoundsComponent>("Global Sounds Library", entity, [](auto& component, auto entity)
		{


			auto& sounds_vector = component->Sounds;

			for (size_t i = 0; i < sounds_vector.size(); ++i)
			{
				auto sound = sounds_vector[i];

				// Construct unique identifiers
				std::stringstream idStream;
				idStream << i;
				std::string idStr = idStream.str();

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), sound->GetTitle().c_str(), sizeof(buffer));

				if (ImGui::InputText(("##SoundName" + idStr).c_str(), buffer, sizeof(buffer)))
				{
					sound->SetTitle(std::string(buffer));
				}

				ImGui::Text("Path: ");
				ImGui::SameLine();
				ImGui::TextWrapped(AssetManager::GetMetadata(sound->GetSoundAsset()).FilePath.string().c_str());

				bool looping = sound->IsLooping();
				if (ImGui::Checkbox(("Looping##" + idStr).c_str(), &looping))
				{
					sound->SetLooping(looping);
				}

				float volume[1] = { sound->GetVolume() };
				ImGui::Text("Sound volume:");
				if (ImGui::SliderFloat(("##sound_volume" + idStr).c_str(), volume, 0.0f, 10.0f))
				{
					sound->SetVolume(volume[0]);
				}

				if (sound->IsPlaying())
				{
					if (ImGui::Button(("Stop##" + idStr).c_str()))
					{
						sound->StopSound();
					}
				}
				else
				{
					if (ImGui::Button(("Play Preview##" + idStr).c_str()))
					{
						sound->PlaySound(true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(("Remove##" + idStr).c_str()))
				{
					sounds_vector.erase(sounds_vector.begin() + i);
					--i;
				}
			}

			DrawDropBox("Add sound file here");
		});

		if (ImGui::BeginDragDropTarget() && m_SelectionContext != 0)
		{
			// Check for internal drag-and-drop payloads
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const auto payload_path = *(const std::filesystem::path*)payload->Data;
				if (DesignAssetManager::GetAssetTypeFromFileExtension(payload_path.extension()) == AssetType::Audio)
				{
					auto entity = m_Scene->GetEntity(m_SelectionContext);
					const auto payload_path = *(const std::filesystem::path*)payload->Data;
					Project::GetActiveDesignAssetManager()->ImportAsset(payload_path);
					auto handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(payload_path);
					auto audioSource = AssetManager::GetAsset<AudioAsset>(handle);

					GlobalSoundsComponent* component;
					if (!entity->HasComponent<GlobalSoundsComponent>())
					{
						entity->AddComponent<GlobalSoundsComponent>(GlobalSoundsComponent());
						
					}
					component = entity->GetComponent<GlobalSoundsComponent>();
					component->Sounds.push_back(CreateRef<GlobalSource>(audioSource->Handle));
				}
			}
			ImGui::EndDragDropTarget();
		 }


		DrawComponent<LocalSoundsComponent>("Local Sounds Library", entity, [](auto& component, auto entity)
		{
			auto& sounds_vector = component->Sounds;

			for (size_t i = 0; i < sounds_vector.size(); ++i)
			{
				auto sound = sounds_vector[i];

				// Construct unique identifiers
				std::stringstream idStream;
				idStream << i;
				std::string idStr = idStream.str();

				char buffer[256];
				memset(buffer, 0, sizeof(buffer));
				strncpy_s(buffer, sizeof(buffer), sound->GetTitle().c_str(), sizeof(buffer));

				if (ImGui::InputText(("##SoundName" + idStr).c_str(), buffer, sizeof(buffer)))
				{
					sound->SetTitle(std::string(buffer));
				}

				ImGui::Text("Path: ");
				ImGui::SameLine();
				ImGui::TextWrapped(AssetManager::GetMetadata(sound->GetSoundAsset()).FilePath.string().c_str());

				bool looping = sound->IsLooping();
				if (ImGui::Checkbox(("Looping##" + idStr).c_str(), &looping))
				{
					sound->SetLooping(looping);
				}

				float volume[1] = { sound->GetVolume() };
				ImGui::Text("Sound volume:");
				if (ImGui::SliderFloat(("##sound_volume" + idStr).c_str(), volume, 0.0f, 10.0f))
				{
					sound->SetVolume(volume[0]);
				}

				float rolloff[1] = { sound->GetRolloff() };
				ImGui::Text("Sound rolloff factor:");
				if (ImGui::SliderFloat(("##sound_rolloff" + idStr).c_str(), rolloff, 0.0f, 1.0f))
				{
					sound->SetRolloff(rolloff[0]);
				}

				if (sound->IsPlaying())
				{
					if (ImGui::Button(("Stop##" + idStr).c_str()))
					{
						sound->StopSound();
					}
				}
				else
				{
					if (ImGui::Button(("Play Preview##" + idStr).c_str()))
					{
						glm::vec3 camera_pos = GetScene()->GetCurrentCamera()->CalculatePosition();
						sound->PlaySound(camera_pos, true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(("Remove##" + idStr).c_str()))
				{
					sounds_vector.erase(sounds_vector.begin() + i);
					--i;
				}
			}

			DrawDropBox("Add sound file here");
		});

		if (ImGui::BeginDragDropTarget() && m_SelectionContext != 0)
		{
			// Check for internal drag-and-drop payloads
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const auto payload_path = *(const std::filesystem::path*)payload->Data;
				if (DesignAssetManager::GetAssetTypeFromFileExtension(payload_path.extension()) == AssetType::Audio)
				{
					auto entity = m_Scene->GetEntity(m_SelectionContext);
					const auto payload_path = *(const std::filesystem::path*)payload->Data;
					Project::GetActiveDesignAssetManager()->ImportAsset(payload_path);
					auto handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(payload_path);
					auto audioSource = AssetManager::GetAsset<AudioAsset>(handle);

					LocalSoundsComponent* component;
					if (!entity->HasComponent<LocalSoundsComponent>())
					{
						entity->AddComponent<LocalSoundsComponent>(LocalSoundsComponent());

					}
					component = entity->GetComponent<LocalSoundsComponent>();
					component->Sounds.push_back(CreateRef<LocalSource>(audioSource->Handle));
				}
			}

			ImGui::EndDragDropTarget();
		}
		

		DrawComponent<MeshComponent>("Mesh", entity, [](auto& component, auto entity)
		{
			if (component->mesh->GetMeshSource())
			{
				const auto& mesh_relative_path = AssetManager::GetMetadata(component->mesh->GetMeshSource()->Handle).FilePath;
				ImGui::Columns(2);
				ImGui::SetColumnWidth(0, 100.f);
				ImGui::Text("Current Mesh");
				if (ImGui::IsItemHovered() && component->mesh)
				{
					ImGui::SetTooltip("%s", Project::GetFullFilePath(mesh_relative_path).string().c_str());
				}
				ImGui::NextColumn();
				ImGui::Text(mesh_relative_path.string().c_str());
				if (ImGui::IsItemHovered() && component->mesh)
				{
					ImGui::SetTooltip("%s", mesh_relative_path.string().c_str());
				}
				ImGui::Columns(1);
			}

			DrawDropBox("Drop mesh here to change");

		});


		if (ImGui::BeginDragDropTarget() && m_SelectionContext != 0)
		{
			// Check for internal drag-and-drop payloads
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				const auto payload_path = *(const std::filesystem::path*)payload->Data;
				if (DesignAssetManager::GetAssetTypeFromFileExtension(payload_path.extension()) == AssetType::MeshSource)
				{
					auto entity = m_Scene->GetEntity(m_SelectionContext);
					const auto payload_path = *(const std::filesystem::path*)payload->Data;
					Project::GetActiveDesignAssetManager()->ImportAsset(payload_path);
					auto handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(payload_path);
					auto meshSource = AssetManager::GetAsset<MeshSource>(handle);

					if (entity->HasComponent<MeshComponent>())
					{
						auto component = entity->GetComponent<MeshComponent>();
						component->mesh->SetMeshSource(meshSource);
					}
					else
					{
						MeshComponent new_comp(CreateRef<Mesh>(meshSource));
						entity->AddComponent<MeshComponent>(new_comp);
					}
				}
			}

			// Check for external file payloads
			//else if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("FILE_PATH"))
			//{
			//	auto meshSource = AssetManager::ImportMesh(payload_path); // Import or load the mesh

			//	if (meshSource)
			//	{
			//		component->mesh->SetMeshSource(meshSource);
			//	}
			//}

			ImGui::EndDragDropTarget();
		}
    }


	template<typename T>
	void EditorPanels::SceneGraph::ShowMapSearchPopup(std::unordered_map<std::string, T>& map, bool use_first, std::string* result_destination)
	{

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
