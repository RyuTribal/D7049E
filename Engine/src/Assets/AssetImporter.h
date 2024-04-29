#pragma once
#include "Asset.h"
#include "AssetMetadata.h"

namespace Engine {
	class AssetImporter
	{
	public:
		static Ref<Asset> Import(AssetHandle handle, const AssetMetadata& metadata);
	};
}
