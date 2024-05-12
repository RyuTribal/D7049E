#pragma once

#include "AssetManagerBase.h"
#include "AssetMetadata.h"
#include "AssetRegistry.h"

namespace Engine {

	class DesignAssetManager : public AssetManagerBase
	{
	public:
		Ref<Asset> GetAsset(AssetHandle handle) override;
		bool IsAssetHandleValid(AssetHandle handle) const override;
		bool IsAssetLoaded(AssetHandle handle) const override;
		const AssetMetadata& GetMetadata(AssetHandle handle) const override;

		static AssetType GetAssetTypeFromFileExtension(const std::filesystem::path& extension);
		static const std::string GetFileExtensionFromAssetType(const AssetType type);

		AssetHandle ImportAsset(const std::filesystem::path& file_path);
		template<typename TAsset, typename... TArgs>
		AssetHandle CreateMemoryOnlyAsset(TArgs&&... args)
		{
			Ref<TAsset> asset = CreateRef<TAsset>(std::forward<TArgs>(args)...);
			AssetMetadata metadata{};
			metadata.FilePath = "";
			metadata.IsEmbedded = true;
			m_AssetRegistry[asset->Handle] = metadata;
			m_LoadedAssets[asset->Handle] = asset;
			return asset->Handle;
		}

		void RegisterAsset(AssetHandle handle, const std::filesystem::path& file_path);
		void UnregsiterAsset(AssetHandle handle);

		bool IsAssetRegistered(const std::filesystem::path& file_path);
		AssetHandle GetHandleByPath(const std::filesystem::path& file_path);

		void SerializeAssetRegistry();
		void DeserializeAssetRegistry();

	private:
		AssetRegistry m_AssetRegistry;
		AssetMap m_LoadedAssets;
	};
}
