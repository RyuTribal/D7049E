#pragma once

namespace YAML {
	class Emitter;
	class Node;
}

namespace Engine {

	struct ProjectSettings;

	struct PremakeProjectConfig
	{
		std::string Name;
		std::string Namespace;
		std::string Files;
	};

	

	class ProjectSerializer
	{
	public:
		static std::pair<bool, std::filesystem::path> CreateNewProject(const std::filesystem::path& directory, const std::string& name);
		static void Serializer(ProjectSettings& settings);
		static ProjectSettings Deserializer(const std::filesystem::path& filepath);
		static void CreateScriptProject();
		static void CreatePremakeFile(const PremakeProjectConfig& config, std::ostream& os);
	};
}
