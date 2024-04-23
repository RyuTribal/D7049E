#include "pch.h"
#include "ProjectSerializer.h"
#include "Scene/SceneSerializer.h"
#include "Serialization/YAMLSerializer.h"
#include "Scene/Scene.h"
#include "Assets/AssetTypes.h"
#include "Assets/AssetManager.h"
#include "Project.h"
#include "Assets/DesignAssetManager.h"

namespace Engine {

	static std::string file_extension = ".hveproject";

	std::pair<bool, std::filesystem::path> ProjectSerializer::CreateNewProject(const std::filesystem::path& directory, const std::string& name)
	{
		if (!std::filesystem::exists(directory))
		{
			HVE_CORE_ERROR_TAG("Project Serializer", "Directory does not exist!");
			return { false, "" };
		}

		std::filesystem::path full_dir_path = directory / std::filesystem::path(name);
		if (std::filesystem::exists(full_dir_path))
		{
			HVE_CORE_ERROR_TAG("Project Serializer", "The folder {0} already exists!", std::filesystem::absolute(full_dir_path).string());
			return { false, "" };
		}


		std::filesystem::create_directory(full_dir_path);
		std::filesystem::path asset_path = full_dir_path / "Assets";
		std::filesystem::create_directory(asset_path);

		std::vector<std::string> sub_dirs = { "Meshes", "Scenes" };

		for (auto& dir : sub_dirs)
		{
			std::filesystem::path new_dir = asset_path / dir;
			std::filesystem::create_directory(new_dir);
		}

		ProjectSettings settings{};
		settings.ProjectName = name;
		settings.RootPath = full_dir_path;
		settings.AssetPath = std::filesystem::path("Assets");
		auto default_scene = Scene::CreateScene("Main_Scene");
		SceneSerializer::Serializer(asset_path / "Scenes", default_scene.get());
		settings.StartingScene = default_scene->Handle;

		auto new_project = CreateRef<Project>(settings);
		Project::SetActive(new_project);
		auto asset_manager = new_project->GetDesignAssetManager();
		std::filesystem::path scene_file_path = asset_path / std::filesystem::path("Scenes") / std::filesystem::path("Main_Scene.hvescn");
		asset_manager->RegisterAsset(default_scene->Handle, scene_file_path);
		Serializer(settings);
		return { true, settings.RootPath / std::filesystem::path(settings.ProjectName + file_extension) };
	}
	void ProjectSerializer::Serializer(ProjectSettings& settings)
	{		
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << settings.ProjectName;
		out << YAML::Key << "AssetPath" << YAML::Value << settings.AssetPath.string();
		out << YAML::Key << "AssetRegistry" << YAML::Value << settings.AssetRegistryPath.string();
		out << YAML::Key << "StartingScene" << YAML::Value << settings.StartingScene;
		out << YAML::EndMap;

		std::filesystem::path filepath = settings.RootPath / std::filesystem::path(settings.ProjectName + file_extension);
		std::ofstream fout(filepath.string());
		HVE_CORE_ASSERT(fout, "Failed to open file for writing: {0}", filepath.string());

		fout << out.c_str();
		fout.close();
		HVE_CORE_ASSERT(!fout.fail(), "Failed to write data to file: {0}", filepath.string());
		HVE_CORE_TRACE_TAG("Project Serializer", "Project saved successfully to: {0}", filepath.string());

	}
	ProjectSettings ProjectSerializer::Deserializer(const std::filesystem::path& filepath)
	{
		YAML::Node config = YAML::LoadFile(filepath.string());
		ProjectSettings settings{};
		const std::filesystem::path root_path = filepath.parent_path();
		settings.ProjectName = config["Project"].as<std::string>();
		settings.RootPath = root_path;
		settings.AssetPath = std::filesystem::path(config["AssetPath"].as<std::string>());
		if (config["AssetRegistry"])
		{
			settings.AssetRegistryPath = std::filesystem::path(config["AssetRegistry"].as<std::string>());
		}
		settings.StartingScene = config["StartingScene"].as<AssetHandle>(0);

		return settings;
	}
}
