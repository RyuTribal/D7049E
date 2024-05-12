#include "pch.h"
#include "ProjectSerializer.h"
#include "Scene/SceneSerializer.h"
#include "Serialization/YAMLSerializer.h"
#include "Scene/Scene.h"
#include "Assets/AssetTypes.h"
#include "Assets/AssetManager.h"
#include "Project.h"
#include "Assets/DesignAssetManager.h"
#include "Core/IO.h"

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

		std::vector<std::string> sub_dirs = { "Meshes", "Scenes", "Scripts", "Meshes/Primitives", "Textures"};

		for (auto& dir : sub_dirs)
		{
			std::filesystem::path new_dir = asset_path / dir;
			std::filesystem::create_directory(new_dir);
		}


		ProjectSettings settings{};
		settings.ProjectName = name;
		settings.RootPath = full_dir_path;
		settings.AssetPath = std::filesystem::path("Assets");
		settings.ScriptAssemblyPath = std::filesystem::path("Binaries/" + name + ".dll");

		auto new_project = CreateRef<Project>(settings);
		Project::SetActive(new_project);
		auto asset_manager = new_project->GetDesignAssetManager();

		auto default_scene = Scene::CreateScene("Main_Scene");
		std::filesystem::path root_path = ROOT_PATH;
		root_path = root_path.parent_path();
		std::filesystem::path default_skybox_path = root_path / "Editor/Resources/Images/default_skybox.hdr";
		std::filesystem::path skybox_dest = asset_path / "Textures/default_skybox.hdr";
		std::filesystem::copy_file(default_skybox_path, skybox_dest, std::filesystem::copy_options::skip_existing);
		auto skybox_handle = Project::GetActiveDesignAssetManager()->ImportAsset(skybox_dest);
		SkyboxSettings skybox{};
		skybox.Texture = AssetManager::GetAsset<TextureCube>(skybox_handle);
		default_scene->SetSkybox(skybox);


		SceneSerializer::Serializer(asset_path / "Scenes", default_scene.get());
		settings.StartingScene = default_scene->Handle;

		std::filesystem::path scene_file_path = asset_path / std::filesystem::path("Scenes") / std::filesystem::path("Main_Scene.hvescn");
		asset_manager->RegisterAsset(default_scene->Handle, scene_file_path);
		CreateScriptProject();

		std::filesystem::path primitives_source = root_path / "Editor/Resources/Primitives";
		std::filesystem::path primitives_destination = asset_path / "Meshes/Primitives";
		for (const auto& entry : std::filesystem::recursive_directory_iterator(primitives_source))
		{
			const auto& primitive_source = entry.path();
			const auto& file = std::filesystem::relative(primitive_source, primitives_source);
			const auto& primitive_destination = primitives_destination / file;
			if (std::filesystem::is_regular_file(primitive_source))
			{
				try
				{
					std::filesystem::copy_file(primitive_source, primitive_destination, std::filesystem::copy_options::skip_existing);
				}
				catch (const std::filesystem::filesystem_error& e)
				{
					HVE_CORE_ERROR("Error copying file: {}", e.what());
				}

				const auto& registration_path = std::filesystem::relative(primitive_destination, asset_path);
				Project::GetActiveDesignAssetManager()->ImportAsset(primitive_destination);
			}
		}

		Serializer(settings);
		return { true, settings.RootPath / std::filesystem::path(settings.ProjectName + file_extension) };
	}

	void ProjectSerializer::Serializer(ProjectSettings& settings)
	{		
		auto renderer_settings = Renderer::Get()->GetSettings();
		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "Project" << YAML::Value << settings.ProjectName;
		out << YAML::Key << "AssetPath" << YAML::Value << settings.AssetPath.string();
		out << YAML::Key << "AssetRegistry" << YAML::Value << settings.AssetRegistryPath.string();
		out << YAML::Key << "StartingScene" << YAML::Value << settings.StartingScene;
		out << YAML::Key << "ScriptAssembly" << YAML::Value << settings.ScriptAssemblyPath.string();
		out << YAML::Key << "Renderer";
		out << YAML::BeginMap;

		out << YAML::Key << "AntiAliasing";
		out << YAML::BeginMap;
		out << YAML::Key << "Type" << YAML::Value << FromAATypeToString(renderer_settings.AntiAliasing.Type);
		out << YAML::Key << "PostProcessing" << YAML::Value << FromPostAATypeToString(renderer_settings.AntiAliasing.PostProcessing);
		out << YAML::Key << "Multiplier" << YAML::Value << renderer_settings.AntiAliasing.Multiplier;
		out << YAML::EndMap;

		out << YAML::EndMap;
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
		if (config["ScriptAssembly"])
		{
			settings.ScriptAssemblyPath = std::filesystem::path(config["ScriptAssembly"].as<std::string>());
		}
		if (config["AssetRegistry"])
		{
			settings.AssetRegistryPath = std::filesystem::path(config["AssetRegistry"].as<std::string>());
		}
		settings.StartingScene = config["StartingScene"].as<AssetHandle>(0);

		if (config["Renderer"])
		{
			if (config["Renderer"]["AntiAliasing"])
			{
				AntiAliasingSettings renderer_aa_settings{};
				renderer_aa_settings.Multiplier = config["Renderer"]["AntiAliasing"]["Multiplier"].as<int>();
				renderer_aa_settings.Type = FromStringToAAType(config["Renderer"]["AntiAliasing"]["Type"].as<std::string>("None"));
				renderer_aa_settings.PostProcessing = FromStringToPostAAType(config["Renderer"]["AntiAliasing"]["PostProcessing"].as<std::string>("None"));
				Renderer::Get()->SetAntiAliasing(renderer_aa_settings);
			}
		}

		return settings;
	}
	void ProjectSerializer::CreateScriptProject()
	{
		auto& project_settings = Project::GetActive()->GetSettings();

		std::filesystem::path external_scripts_path = project_settings.RootPath / "ScriptProject"; // Note this is scripts for building stuff in the project, such as our scripts assembly
		std::filesystem::create_directory(external_scripts_path);

		PremakeProjectConfig config;
		config.Name = project_settings.ProjectName;
		config.Namespace = project_settings.ProjectName;
		config.Files = "../Assets/Scripts/**.cs";
		HVE_CORE_WARN(external_scripts_path.string());
		std::ofstream outFile(external_scripts_path / "premake5.lua");
		CreatePremakeFile(config, outFile);
		outFile.close();

		CommandArgs args{};
		args.SleepUntilFinished = true;
		std::filesystem::path root_path = ROOT_PATH;
		root_path = root_path.parent_path();
#ifdef PLATFORM_WINDOWS:
		std::filesystem::path premake_executable = root_path / std::filesystem::path("vendor/premake/bin/premake5.exe");
		std::string command = premake_executable.string() + " --file=" + project_settings.RootPath.string() + "/ScriptProject/premake5.lua" + " vs2022";
		// Ugly fix
		std::replace(command.begin(), command.end(), '/', '\\');
#endif
		CommandLine::Create()->ExecuteCommand(command, args);

		project_settings.ScriptAssemblyPath = std::filesystem::path("Binaries/" + project_settings.ProjectName + ".dll");
	}
	void ProjectSerializer::CreatePremakeFile(const PremakeProjectConfig& config, std::ostream& os)
	{

		// Yep, this is happening...

		os << "include(os.getenv(\"HVE_ROOT_DIR\") .. \"/vendor/premake/premake_customization/solution_items.lua\")\n";

		os << "workspace \"" << config.Name << "\"\n";
		os << "architecture \"x86_64\"\n";
		os << "startproject \"" << config.Name << "\"\n";
		os << "configurations { \"Debug\", \"Release\", \"Dist\" }\n";
		os << "flags {\n";
		os << "\t\"MultiProcessorCompile\",\n";
		os << "}\n";

		os << "project \"" << config.Name << "\"\n";
		os << "kind \"SharedLib\"\n";
		os << "language \"C#\"\n";
		os << "dotnetframework \"4.7.2\"\n";
		os << "targetdir \"../Binaries\"\n";
		os << "objdir \"../Binaries/Intermediates\"\n";
		os << "namespace \"" << config.Namespace << "\"\n";
		os << "location" << "\"../Assets/Scripts\"\n";

		os << "links {\n";
		os << "\tos.getenv(\"HVE_ROOT_DIR\") .. \"/Editor/Resources/Scripts/ScriptCore\",\n";
		os << "\tos.getenv(\"HVE_ROOT_DIR\") .. \"/Editor/mono/lib/mono/4.5/System.Numerics\",\n";
		os << "\tos.getenv(\"HVE_ROOT_DIR\") .. \"/Editor/mono/lib/mono/4.5/System.Numerics.Vectors\",\n";
		os << "}\n";

		os << "files {\n";
		os << "\t\"" << config.Files << "\",\n";
		os << "}\n";

		os << "vpaths {\n";
		os << "\t" << "[\"Scripts\"] = {\"" + config.Files + "\"}" << ",\n";
		os << "}\n";

		os << "filter \"configurations:Debug\"\n";
		os << "optimize \"Off\"\n";
		os << "symbols \"Default\"\n";
		os << "filter \"configurations:Release\"\n";
		os << "optimize \"On\"\n";
		os << "symbols \"Default\"\n";
		os << "filter \"configurations:Dist\"\n";
		os << "optimize \"Full\"\n";
		os << "symbols \"Off\"\n";

		os << "group \"Helios\"\n";
		os << "include(os.getenv(\"HVE_ROOT_DIR\") .. \"/ScriptCore\")\n";
	}
}
