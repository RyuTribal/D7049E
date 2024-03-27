#include "pch.h"
#include "Paths.h"

namespace Engine 
{
	const std::unordered_map<std::string, std::string> Paths::paths = 
	{
			{"LineShader", std::string(ROOT_PATH) + "/shaders/line_shader"},
			{"PointShader", std::string(ROOT_PATH) + "/shaders/point_shader"},
	};
}