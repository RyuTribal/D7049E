#pragma once

#include "ProjectSerializer.h"
#include <Assets/AssetManagerBase.h>
#include "Assets/DesignAssetManager.h"

namespace Engine {
	struct ProjectSettings
	{
		std::string ProjectName = "Untitled";
		std::filesystem::path RootPath;
		std::filesystem::path AssetPath;
		std::filesystem::path AssetRegistryPath = std::filesystem::path("AssetRegistry" + DesignAssetManager::GetFileExtensionFromAssetType(AssetType::AssetRegistry)); // relative to asset directory
		std::filesystem::path ScriptAssemblyPath = std::filesystem::path("Binaries") / std::filesystem::path(ProjectName + ".dll");
		AssetHandle StartingScene;
	};


	class Project
	{
	public:

		Project(ProjectSettings settings) : m_Settings(settings), m_DesignAssetManager(CreateRef<DesignAssetManager>())
		{
		
		}

		ProjectSettings& GetSettings() { return m_Settings; }
		static Ref<Project> GetActive() { return s_ActiveProject; }

		static Ref<AssetManagerBase> GetAssetManager() { return s_ActiveProject->m_DesignAssetManager; }
		static Ref<DesignAssetManager> GetActiveDesignAssetManager() { return std::static_pointer_cast<DesignAssetManager>(s_ActiveProject->m_DesignAssetManager); }

		Ref<DesignAssetManager> GetDesignAssetManager() { return std::static_pointer_cast<DesignAssetManager>(m_DesignAssetManager); }

		static Ref<Project> New(std::string& name);
		static Ref<Project> New(std::string& name, std::filesystem::path& file_path);
		static Ref<Project> Load(const std::filesystem::path& file_path);
		static bool SaveActive(const std::filesystem::path& file_path);
		static bool SaveActive();
		static void SetActive(Ref<Project> other_project) { s_ActiveProject = other_project; }
		static std::filesystem::path GetFullFilePath(const std::filesystem::path& relative_file_path);
		static bool ArePathsEqual(const std::filesystem::path& relative_file_path, const std::filesystem::path& another);

		static void CreateScriptProject() { ProjectSerializer::CreateScriptProject(); ProjectSerializer::Serializer(s_ActiveProject->m_Settings); }
		static void ReloadScripts();

	private:
		ProjectSettings m_Settings;
		Ref<AssetManagerBase> m_DesignAssetManager;

		static Ref<Project> s_ActiveProject;
	};
}
