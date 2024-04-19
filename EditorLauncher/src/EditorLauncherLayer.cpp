#include "EditorLauncherLayer.h"
#include <imgui/imgui_internal.h>

namespace EditorLauncher {
	void EditorLauncherLayer::OnAttach()
	{
		memset(name_buffer, 0, sizeof(name_buffer));
		memset(path_buffer, 0, sizeof(path_buffer));
	}
	void EditorLauncherLayer::OnImGuiRender()
	{
		static bool dockspaceOpen = true;
		ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_PassthruCentralNode | ImGuiDockNodeFlags_NoTabBar;

		ImGuiWindowFlags window_flags =  ImGuiWindowFlags_NoDocking |
			ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
			ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
			ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus | ImGuiWindowFlags_NoScrollbar;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("DockSpace Demo", &dockspaceOpen, window_flags);
		ImGui::PopStyleVar();

		ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
		ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

		ImGui::SetNextWindowDockID(dockspace_id, ImGuiCond_FirstUseEver);

		ImGui::Begin("Game Engine Launcher");

		ImGui::Columns(2, "LauncherColumns", false);
		ImGui::SetColumnWidth(0, (float)ImGui::GetWindowSize().x/2);

		ImGui::BeginChild("ProjectList", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), window_flags);
		ImGui::Text("Projects");
		/*for (auto& project : projects)
		{
			if (ImGui::Selectable(project.name.c_str(), &project.isSelected))
			{
			}
		}*/
		ImGui::EndChild();

		ImGui::NextColumn();

		ImGui::BeginChild("ActionButtons", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), window_flags); // Adjust height as needed
		float total_button_height = ImGui::GetTextLineHeightWithSpacing() * 2;
		float available_space = ImGui::GetContentRegionAvail().y;
		float start_y_pos = (available_space - total_button_height) * 0.5f;

		if (start_y_pos > 0)
		{
			ImGui::SetCursorPosY(start_y_pos);
		}

		if (ImGui::Button("New Project", ImVec2(-1, 0)))
		{
			ImGui::OpenPopup("Create Project");
		}

		if (ImGui::BeginPopupModal("Create Project"))
		{
			ImGui::SetNextWindowSize({600, 338});
			ImGui::Columns(2, "CreateColumns", false);
			ImGui::SetColumnWidth(0, 200);
			ImGui::Text("Project Name");
			ImGui::Text("Project Directory");
			ImGui::NextColumn();

			ImGui::InputText("##Name", name_buffer, sizeof(name_buffer));
			ImGui::InputText("##Dir", path_buffer, sizeof(path_buffer));
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				std::string path = Engine::FilePicker::OpenFileExplorer(true);
				strncpy(path_buffer, path.c_str(), sizeof(path_buffer));
				path_buffer[sizeof(path_buffer) - 1] = '\0';
			}

			ImGui::Columns(1);

			bool success = false;
			bool attempted_to_create = false;

			if (ImGui::Button("Create"))
			{
				auto [success, project_file_path] = Engine::ProjectSerializer::CreateNewProject(path_buffer, name_buffer);
				attempted_to_create = true;
				if (success)
				{
					OpenProject(project_file_path);
				}
			}

			ImGui::SameLine();
			if (ImGui::Button("Cancel"))
			{
				ImGui::CloseCurrentPopup();
			}

			if (!success && attempted_to_create)
			{
				ImGui::Text("Error: Project creation failed. Please check the logs!");
			}

			ImGui::EndPopup();
		}

		if (ImGui::Button("Open Project", ImVec2(-1, 0)))
		{
			std::string file_ending = Engine::Utils::GetFileEndings(Engine::ProjectAsset);
			file_ending.erase(file_ending.begin()); // Have to remove the . for it to work
			std::vector<std::vector<std::string>> filter = { {"Helios project files", file_ending} };
			std::string path = Engine::FilePicker::OpenFileExplorer(filter, false);
			OpenProject(path);
		}
		ImGui::EndChild();

		ImGui::Columns(1);

		ImGui::End();

		ImGui::End(); // End of DockSpace Demo window
	}
	void EditorLauncherLayer::OnEvent(Engine::Event& event)
	{
		
	}
	void EditorLauncherLayer::OpenProject(std::string& project_path)
	{
		Engine::CommandArgs args{};
		args.NewProcess = true;
		args.UseAnotherWorkingDir = true;
		args.SleepUntilFinished = false;
		args.WorkingDir = std::string(EDITOR_WORKING_DIRECTORY);
		HVE_INFO(args.WorkingDir);
		std::string command = "\"" + std::string(EDITOR_EXECUTABLE_PATH) + "\" \"" + project_path + "\"";
		std::replace(command.begin(), command.end(), '/', '\\');
		std::replace(args.WorkingDir.begin(), args.WorkingDir.end(), '/', '\\');
		Engine::CommandLine::Create()->ExecuteCommand(command, args);
		Engine::Application::Get().Close();
	}
}
