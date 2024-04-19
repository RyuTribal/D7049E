#pragma once

#include "Core/ProjectSettings.h"

namespace YAML {
	class Emitter;
	class Node;
}

namespace Engine {
	class ProjectSerializer
	{
	public:
		static std::pair<bool, std::string> CreateNewProject(const std::string& directory, const std::string& name);
		static void Serializer(ProjectSettings& settings);
		static ProjectSettings Deserializer(const std::string& filepath);
	};
}
