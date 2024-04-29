#pragma once

#include "AssetManagerBase.h"
#include "Project/Project.h"

namespace Engine {

	class AssetManager
	{
	public:
		template<typename T>
		static Ref<T> GetAsset(AssetHandle handle)
		{
			// Get the project asset manager
			Ref<Asset> asset = Project::GetAssetManager()->GetAsset(handle);
			return std::static_pointer_cast<T>(asset);
		}

		static const AssetMetadata& GetMetadata(AssetHandle handle)
		{
			const AssetMetadata& metadata = Project::GetAssetManager()->GetMetadata(handle);
			return metadata;
		}
		
	};
}
