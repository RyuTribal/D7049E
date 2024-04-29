#include "pch.h"
#include "Project.h"
#include "UI/FilePicker.h"
#include "Assets/DesignAssetManager.h"

namespace Engine {
	Ref<Project> Project::s_ActiveProject = nullptr;
	Ref<Project> Project::New(std::string& name)
	{
		std::string path = FilePicker::OpenFileExplorer(true);
		auto [success, project_file_path] = ProjectSerializer::CreateNewProject(path, name);
		HVE_CORE_ASSERT(success, "Something went wrong with the project creation!");

		return Load(project_file_path);
	}

	Ref<Project> Project::New(std::string& name, std::filesystem::path& file_path)
	{
		auto [success, project_file_path] = ProjectSerializer::CreateNewProject(file_path, name);
		HVE_CORE_ASSERT(success, "Something went wrong with the project creation!");

		return Load(project_file_path);
	}

	Ref<Project> Project::Load(const std::filesystem::path& file_path)
	{
		ProjectSettings settings = ProjectSerializer::Deserializer(file_path);
		s_ActiveProject = CreateRef<Project>(settings);
		s_ActiveProject->GetDesignAssetManager()->DeserializeAssetRegistry();

		return s_ActiveProject;
	}
	bool Project::SaveActive(const std::filesystem::path& file_path)
	{
		HVE_CORE_ASSERT(s_ActiveProject, "No active project exists!");
		s_ActiveProject->GetSettings().RootPath = file_path;
		ProjectSerializer::Serializer(s_ActiveProject->GetSettings());
		return true; // Should probably have a way to check if it saved correctly
	}
	bool Project::SaveActive()
	{
		HVE_CORE_ASSERT(s_ActiveProject, "No active project exists!");
		ProjectSerializer::Serializer(s_ActiveProject->GetSettings());
		return true;  // Should probably have a way to check if it saved correctly
	}
	std::filesystem::path Project::GetFullFilePath(const std::filesystem::path& relative_file_path)
	{
		return s_ActiveProject->GetSettings().RootPath / relative_file_path;
	}
}
