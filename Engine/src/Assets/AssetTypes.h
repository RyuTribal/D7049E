#pragma once
#include <string>

namespace Engine{
	enum AssetType
	{
		ProjectAsset,
		SceneAsset
	};

	namespace Utils {
		static std::string GetFileEndings(AssetType type)
		{
			switch (type)
			{
				case ProjectAsset:      return ".hveproject";
				case SceneAsset:        return ".hvescn";
			}
		}
	}
}
