#include "pch.h"
#include "Project.h"
#include "UI/FilePicker.h"
#include "Assets/DesignAssetManager.h"
#include "Script/ScriptEngine.h"

namespace Engine {
	Ref<Project> Project::s_ActiveProject = nullptr;
	Ref<Project> Project::New(std::string& name)
	{
		// Should probably remove this function...
		auto [success, path] = FilePicker::OpenFileExplorer(true);
		if (success)
		{
			auto [success_proj, project_file_path] = ProjectSerializer::CreateNewProject(path, name);
			HVE_CORE_ASSERT(success_proj, "Something went wrong with the project creation!");
			return Load(project_file_path);
		}

		HVE_CORE_ASSERT(success, "Something went wrong with the project creation!");
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
		std::filesystem::path assembly_full_path = settings.RootPath / settings.ScriptAssemblyPath;

		ScriptEngine::LoadAppAssembly(assembly_full_path);

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
	bool Project::ArePathsEqual(const std::filesystem::path& relative_file_path, const std::filesystem::path& another)
	{
		if (relative_file_path.is_absolute())
		{
			return relative_file_path == another;
		}

		return s_ActiveProject->m_Settings.RootPath / relative_file_path == another;
	}
	void Project::ReloadScripts()
	{
		std::filesystem::path assembly_path = s_ActiveProject->m_Settings.RootPath / s_ActiveProject->m_Settings.ScriptAssemblyPath;
		if (std::filesystem::exists(assembly_path))
		{
			ScriptEngine::ReloadAssembly(s_ActiveProject->m_Settings.RootPath / s_ActiveProject->m_Settings.ScriptAssemblyPath);
		}
	}
}
