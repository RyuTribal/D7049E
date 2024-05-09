#include "pch.h"

#include "AssetImporter.h"
#include "TextureImporter.h"
#include "ModelImporter.h"
#include "AudioImporter.h"
#include "Scene/Scene.h"

namespace Engine {

	using AssetImportFunction = std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>;
	static std::map <AssetType, std::function<Ref<Asset>(AssetHandle, const AssetMetadata&)>> s_AssetLoadingFunctions = 
	{
		// Project asset is excluded because the asset manager depends on it so we have the project loader managed by the project file itself (since no more than one project will ever be loaded in a project...)
		{AssetType::Scene, Scene::LoadScene},
		{AssetType::Texture, TextureImporter::Import2D},
		{AssetType::CubeMap, TextureImporter::ImportCube},
		{AssetType::MeshSource, ModelImporter::ImportSource},
		{AssetType::Audio, AudioImporter::Import},
		//{AssetType::Mesh, ModelImporter::Import},
		/*{AssetType::Material, },*/
	};

	Ref<Asset> AssetImporter::Import(AssetHandle handle, const AssetMetadata& metadata)
	{
		if(!s_AssetLoadingFunctions.contains(metadata.Type))
		{
			HVE_CORE_ERROR_TAG("Asset Importer", "Wow! Tried to use an importer for a type that isn't defined!");
			return nullptr;
		}
		Ref<Asset> new_asset = s_AssetLoadingFunctions.at(metadata.Type)(handle, metadata);
		if (!new_asset)
		{
			HVE_CORE_ERROR_TAG("Asset Importer", "Something went wrong here!");
			return nullptr;
		}
		new_asset->Handle = handle;
		return new_asset;
	}
}
