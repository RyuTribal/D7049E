#pragma once
#include <Sound/AudioAsset.h>
#include "AssetMetadata.h"


namespace Engine {
	class AudioImporter
	{
	public:
		static Ref<AudioAsset> Import(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<AudioAsset> ImportByFilePath(const std::filesystem::path& file_path);
	};
}
