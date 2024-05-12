#pragma once
#include "Renderer/Texture.h"
#include "AssetMetadata.h"


namespace Engine {
	class TextureImporter
	{
	public:
		static Ref<Texture2D> Import2D(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Texture2D> Import2DWithPath(const std::string& path);

		static Ref<TextureCube> ImportCube(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<TextureCube> ImportCubeWithPath(const std::string& path);
	};
}
