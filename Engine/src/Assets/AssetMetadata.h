#pragma once

#include "Asset.h"

namespace Engine {
	struct AssetMetadata
	{
		AssetType Type = AssetType::None;
		std::filesystem::path FilePath;
		bool IsEmbedded = false;

		operator bool() const { return Type == AssetType::None; }
	};
}
