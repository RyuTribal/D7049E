#pragma once
#include "Assets/Asset.h"


namespace Engine {
	class AudioAsset : public Asset
	{
	public:
		AudioAsset(const std::filesystem::path& audio_path): m_AudioPath(audio_path) {}

		static AssetType GetStaticType() { return AssetType::Audio; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

	private:
		std::filesystem::path m_AudioPath;
	};
}
