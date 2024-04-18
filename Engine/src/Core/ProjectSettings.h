#pragma once
#include <string>

namespace Engine {
	struct ProjectSettings
	{
		std::string ProjectName;
		std::string RootPath;
		std::string AssetPath;
		std::string StartingScene;
	};
}
