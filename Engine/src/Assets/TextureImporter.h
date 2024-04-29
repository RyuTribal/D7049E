#pragma once
#include "Renderer/Texture.h"
#include "AssetMetadata.h"


namespace Engine {
	class TextureImporter
	{
	public:
		static Ref<Texture2D> Import(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> ImportWithPath(const std::string& path);
	};
}
