#include "ProjectSettings.h"
#include <imgui/imgui_internal.h>
#include <imgui/imgui.h>


namespace EditorPanels {

	struct ProjectData
	{
		bool UseAA = false;

		Engine::AntiAliasingSettings AASettings;

		bool HasChanged = false;
	};

	static ProjectData* s_InstanceData = nullptr;

	template<typename UIFunction>
	static void DrawSection(const std::string& name, UIFunction uiFunction)
	{
		const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowItemOverlap | ImGuiTreeNodeFlags_FramePadding;

		ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();
		float lineHeight = GImGui->Font->FontSize + GImGui->Style.FramePadding.y * 2.0f;

		ImGui::Separator(); // Consider repositioning if needed.

		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
		bool open = ImGui::TreeNodeEx(("##project_settings_" + name).c_str(), treeNodeFlags, name.c_str());
		ImGui::PopStyleVar();

		if (open)
		{
			uiFunction();
			ImGui::TreePop();
		}
	}

	template<typename UIFunction>
	static void DrawOption(const std::string& label, UIFunction uiFunction)
	{
		ImGui::Columns(2);
		ImGui::SetColumnWidth(0, 150.f);
		ImGui::Text(label.c_str());
		ImGui::NextColumn();
		uiFunction();
		ImGui::Columns(1);
	}

	void ProjectSettings::Init()
	{
		s_InstanceData = new ProjectData();
		s_InstanceData->AASettings = Engine::Renderer::Get()->GetSettings().AntiAliasing;
		s_InstanceData->UseAA = s_InstanceData->AASettings.Type != Engine::AAType::None;
	}

	void ProjectSettings::Render(Engine::Ref<Engine::Camera> editor_camera)
	{
		DrawSection("Editor camera", [editor_camera]() {

			const char* projectionTypeStrings[] = { "Perspective", "Orthographic" };
			const char* currentProjectionTypeString = editor_camera->GetType() == Engine::CameraType::PERSPECTIVE ? "Perspective" : "Orthographic";
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
						editor_camera->ChangeCameraType(i == 0 ? Engine::CameraType::PERSPECTIVE : Engine::CameraType::ORTHOGRAPHIC);
					}

					if (isSelected)
						ImGui::SetItemDefaultFocus();
				}

				ImGui::EndCombo();
			}
			ImGui::Columns(1);
			float perspectiveVerticalFov = editor_camera->GetFOVY();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Vertical FOV");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##fov", &perspectiveVerticalFov))
				editor_camera->SetFovy(perspectiveVerticalFov);

			ImGui::Columns(1);
			float perspectiveNear = editor_camera->GetNear();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Near");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##near", &perspectiveNear))
				editor_camera->SetNear(perspectiveNear);
			ImGui::Columns(1);
			float perspectiveFar = editor_camera->GetFar();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Far");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##far", &perspectiveFar))
				editor_camera->SetFar(perspectiveFar);

			ImGui::Columns(1);

			float camera_zoom = editor_camera->GetZoomDistance();
			ImGui::Columns(2);
			ImGui::SetColumnWidth(0, 100.f);
			ImGui::Text("Zoom");
			ImGui::NextColumn();
			if (ImGui::DragFloat("##zoom", &camera_zoom))
				editor_camera->SetZoomDistance(camera_zoom);
			ImGui::Columns(1);
		
		});

		DrawSection("Anti Aliasing", []() {

			DrawOption("Use Antialiasing", []() {
				if (ImGui::Checkbox("##use_antialiasing_checkbox", &s_InstanceData->UseAA))
				{
					s_InstanceData->AASettings.Type = Engine::AAType::MSAA;
					s_InstanceData->HasChanged = true;
				}
			});

			ImGui::BeginDisabled(!s_InstanceData->UseAA);


			DrawOption("Type", []() {
				std::string type_string = FromAATypeToString(s_InstanceData->AASettings.Type);
				if (ImGui::BeginCombo("##AATypes", type_string.c_str()))
				{
					bool is_selected = s_InstanceData->AASettings.Type == Engine::AAType::SSAA;
					if (ImGui::Selectable("Super Sampling", is_selected))
					{
						s_InstanceData->AASettings.Type = Engine::AAType::SSAA;
						s_InstanceData->HasChanged = true;
					}

					is_selected = s_InstanceData->AASettings.Type == Engine::AAType::MSAA;
					if (ImGui::Selectable("Multi Sampling", is_selected))
					{
						s_InstanceData->AASettings.Type = Engine::AAType::MSAA;
						s_InstanceData->HasChanged = true;
					}
					ImGui::EndCombo();
				}
			});

			/*DrawOption("Post Processesing", []() {
				std::string type_string = FromPostAATypeToString(s_InstanceData->AASettings.PostProcessing);
				if (ImGui::BeginCombo("##PPAATypes", type_string.c_str()))
				{
					bool is_selected = s_InstanceData->AASettings.PostProcessing == Engine::PPAAType::None;
					if (ImGui::Selectable("None", is_selected))
					{
						s_InstanceData->AASettings.PostProcessing = Engine::PPAAType::None;
						s_InstanceData->HasChanged = true;
					}

					is_selected = s_InstanceData->AASettings.PostProcessing == Engine::PPAAType::FXAA;
					if (ImGui::Selectable("FXAA", is_selected))
					{
						s_InstanceData->AASettings.PostProcessing = Engine::PPAAType::FXAA;
						s_InstanceData->HasChanged = true;
					}

					ImGui::EndCombo();
				}
			});*/

			DrawOption("Multiplier", []() {
				if (ImGui::DragInt("##AAMultiplier", (int*)&s_InstanceData->AASettings.Multiplier, 2.f, 2, 8))
				{
					s_InstanceData->HasChanged = true;
				}
			});

			ImGui::EndDisabled();
		});

		if (s_InstanceData->HasChanged)
		{
			s_InstanceData->AASettings.Type = s_InstanceData->UseAA ? s_InstanceData->AASettings.Type : Engine::AAType::None;

			Engine::Renderer::Get()->SetAntiAliasing(s_InstanceData->AASettings);
			s_InstanceData->HasChanged = false;
		}
	
	}
	void ProjectSettings::Shutdown()
	{
		delete s_InstanceData;
	}
}
