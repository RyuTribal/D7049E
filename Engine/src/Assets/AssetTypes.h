#pragma once
#include <string>

namespace Engine{
	enum class AssetType : uint16_t
	{
		None = 0,
		Project,
		AssetRegistry,


		Scene,
		Mesh,
		MeshSource,
		Material,
		Audio,
		Texture
	};

	namespace Utils {

		static std::string AssetTypeToString(AssetType type)
		{
			switch (type)
			{
				case AssetType::None:		  return "None";
				case AssetType::Scene:        return "Scene";
				case AssetType::AssetRegistry:return "AssetRegistry";
				case AssetType::Mesh:         return "Mesh";
				case AssetType::MeshSource:   return "MeshSource";
				case AssetType::Material:     return "Material";
				case AssetType::Audio:        return "Audio";
				case AssetType::Texture:      return "Texture";
			}
		}

		static AssetType StringToAssetType(const std::string& type_string)
		{
			if (type_string == "None")                return AssetType::None;
			if (type_string == "Scene")               return AssetType::Scene;
			if (type_string == "Mesh")                return AssetType::Mesh;
			if (type_string == "MeshSource")          return AssetType::MeshSource;
			if (type_string == "Material")            return AssetType::Material;
			if (type_string == "Texture")             return AssetType::Texture;
			if (type_string == "Audio")               return AssetType::Audio;
		}
	}
}
