#pragma once

namespace YAML {
	class Emitter;
	class Node;
}

namespace Engine {

	struct ProjectSettings;

	class ProjectSerializer
	{
	public:
		static std::pair<bool, std::filesystem::path> CreateNewProject(const std::filesystem::path& directory, const std::string& name);
		static void Serializer(ProjectSettings& settings);
		static ProjectSettings Deserializer(const std::filesystem::path& filepath);
	};
}
