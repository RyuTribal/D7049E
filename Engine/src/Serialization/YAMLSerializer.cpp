#include "pch.h"
#include "YAMLSerializer.h"

namespace Engine {
	YAML::Node YAMLSerializer::Load(const std::string& filepath)
	{
		return YAML::LoadFile(filepath);
	}
	void YAMLSerializer::Save(const YAML::Node& node, const std::string& filepath)
	{
		std::ofstream fout(filepath);
		fout << node;
	}
}
