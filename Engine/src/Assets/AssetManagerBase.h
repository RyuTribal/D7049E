#pragma once

#include "Asset.h"
#include "AssetMetadata.h"

namespace Engine {

	using AssetMap = std::map<AssetHandle, Ref<Asset>>;

	class AssetManagerBase
	{
	public:
		virtual Ref<Asset> GetAsset(AssetHandle handle) = 0;


		virtual bool IsAssetHandleValid(AssetHandle handle) const = 0;
		virtual bool IsAssetLoaded(AssetHandle handle) const = 0;
		virtual const AssetMetadata& GetMetadata(AssetHandle handle) const = 0;
	};
}
