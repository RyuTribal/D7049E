#include "pch.h"
#include "ProjectSerializer.h"
#include "SceneSerializer.h"
#include "YAMLSerializer.h"
#include "Scene/Scene.h"
#include "Assets/AssetTypes.h"

namespace Engine {
	bool ProjectSerializer::CreateNewProject(const std::string& directory, const std::string& name)
	{
		if (!std::filesystem::exists(directory))
		{
			HVE_CORE_ERROR_TAG("Project Serializer", "Directory does not exist!");
			return false;
		}
		char forward_slash = '/';
		char back_slash = '\\';
		std::string full_dir_path = directory;
		if (directory.back() != back_slash || directory.back() != forward_slash)
		{
			full_dir_path += "/"; // Don't really need to check what slash we need to put since std::filesystem seems to take care of that anyway
		}
		full_dir_path += name;
		if (std::filesystem::exists(full_dir_path))
		{
			HVE_CORE_ERROR_TAG("Project Serializer", "The folder {0} already exists!", std::filesystem::absolute(full_dir_path).string());
			return false;
		}

		std::filesystem::path root_path = std::filesystem::absolute(full_dir_path);

		std::filesystem::create_directory(root_path);
		std::filesystem::path asset_path = std::filesystem::path(root_path.string() + "/Assets");
		std::filesystem::create_directory(asset_path);

		std::vector<std::string> sub_dirs = { "/Meshes", "/Scenes" };

		for (auto& dir : sub_dirs)
		{
			std::filesystem::path new_dir = std::filesystem::path(asset_path.string() + dir);
			std::filesystem::create_directory(new_dir);
		}

		ProjectSettings settings{};
		settings.ProjectName = name;
		settings.RootPath = root_path.string();
		settings.AssetPath = "/Assets";
		settings.StartingScene = "Assets/Scenes/Main_Scene"+ Utils::GetFileEndings(AssetType::SceneAsset);
		auto [default_scene, camera_entity_handle] = Scene::CreateScene("Main_Scene");
		SceneSerializer::Serializer(asset_path.string() + "/Scenes/", default_scene);
		Serializer(settings);
		return true;
	}
	void ProjectSerializer::Serializer(ProjectSettings& settings)
	{		
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << settings.ProjectName;
		out << YAML::Key << "AssetPath" << YAML::Value << settings.AssetPath;
		out << YAML::Key << "StartingScene" << YAML::Value << settings.StartingScene;
		out << YAML::EndMap;

		std::string filepath = settings.RootPath + "/" + settings.ProjectName + Utils::GetFileEndings(ProjectAsset);
		std::ofstream fout(filepath);
		HVE_CORE_ASSERT(fout, "Failed to open file for writing: {0}", filepath);

		fout << out.c_str();
		fout.close();
		HVE_CORE_ASSERT(!fout.fail(), "Failed to write data to file: {0}", filepath);
		HVE_CORE_TRACE_TAG("Project Serializer", "Project saved successfully to: {0}", filepath);

	}
	void ProjectSerializer::Deserializer(const std::string& filepath)
	{

	}
}
