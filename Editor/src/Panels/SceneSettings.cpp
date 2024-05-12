#include "SceneSettings.h"
#include "SceneGraph.h"
#include <imgui/imgui_internal.h>

using namespace Engine;

namespace EditorPanels {
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


	void SceneSettings::Render(Ref<Scene> context)
	{
		DrawSection("Skybox", [context]() 
		{
			SkyboxSettings scene_skybox = context->GetSkybox();

			SceneGraph::DrawDropBox("Drop to change skybox");

			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					const auto payload_path = *(const std::filesystem::path*)payload->Data;
					if (DesignAssetManager::GetAssetTypeFromFileExtension(payload_path.extension()) == AssetType::CubeMap)
					{
						auto handle = Project::GetActiveDesignAssetManager()->GetHandleByPath(payload_path);
						if (handle == 0)
						{
							handle = Project::GetActiveDesignAssetManager()->ImportAsset(payload_path);
						}
						scene_skybox.Texture = AssetManager::GetAsset<TextureCube>(handle);
						context->SetSkybox(scene_skybox);
					}
				}
			}

			float Brightness = scene_skybox.Brightness * 100.f;

			DrawOption("Brightness", [&context, &scene_skybox, &Brightness]()
			{
				if(ImGui::DragFloat("##skybox_brightness", &Brightness, 1.f, 0.0f, 1000.f, "%.1f"))
				{
					scene_skybox.Brightness = Brightness / 100.f;
					context->SetSkybox(scene_skybox);
				}
			});
		});
		
	}
}
