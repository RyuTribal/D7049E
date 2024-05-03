#include "pch.h"
#include "AudioImporter.h"
#include "Project/Project.h"

namespace Engine {
	Ref<AudioAsset> AudioImporter::Import(AssetHandle handle, const AssetMetadata& metadata)
	{
		std::filesystem::path file_path = Project::GetFullFilePath(metadata.FilePath);
		Ref<AudioAsset> audio_asset = CreateRef<AudioAsset>(file_path);
		return audio_asset;
	}
	Ref<AudioAsset> AudioImporter::ImportByFilePath(const std::filesystem::path& file_path)
	{
		Ref<AudioAsset> audio_asset = CreateRef<AudioAsset>(file_path);
		return audio_asset;
	}
}
