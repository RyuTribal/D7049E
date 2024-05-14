#include "pch.h"
#include "TextureImporter.h"
#include "Core/Buffer.h"
#include "Project/Project.h"

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine {
	Ref<Texture2D> TextureImporter::Import2D(AssetHandle handle, const AssetMetadata& metadata)
	{
		HVE_PROFILE_FUNC();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = nullptr;
		std::filesystem::path full_path = Project::GetFullFilePath(metadata.FilePath.string().c_str());
		AssetType type = Project::GetActiveDesignAssetManager()->GetAssetTypeFromFileExtension(full_path.extension());
		{
			HVE_PROFILE_SCOPE("stbi_load - TextureImporter::Import");
			if (type == AssetType::CubeMap)
			{
				data.Data = stbi_loadf(full_path.string().c_str(), &width, &height, &channels, 0);
			}
			else
			{
				data.Data = stbi_load(full_path.string().c_str(), &width, &height, &channels, 0);
			}
		}


		if (data.Data == nullptr)
		{
			HVE_CORE_ERROR_TAG("Texture Importer", "Could not load texture for file path: {0}", metadata.FilePath);
			return nullptr;
		}

		data.Size = width * height * channels;

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		switch (channels)
		{
			case 1:
				spec.Format = ImageFormat::R8;
				break;
			case 3:
				spec.Format = type == AssetType::CubeMap ? ImageFormat::RGB32F : ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = type == AssetType::CubeMap ? ImageFormat::RGBA32F : ImageFormat::RGBA8;
				break;
			default:
				HVE_CORE_ASSERT(false, "Image has an invalid format");
		}

		
		
		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		stbi_image_free(data.Data);
		return texture;
	}

	Ref<Texture2D> TextureImporter::Import2DWithPath(const std::string& path)
	{
		HVE_PROFILE_FUNC();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = nullptr;
		std::filesystem::path full_path = path;
		AssetType type = Project::GetActiveDesignAssetManager()->GetAssetTypeFromFileExtension(full_path.extension());
		{
			HVE_PROFILE_SCOPE("stbi_load - TextureImporter::Import");
			if (type == AssetType::CubeMap)
			{
				data.Data = stbi_loadf(full_path.string().c_str(), &width, &height, &channels, 0);
			}
			else
			{
				data.Data = stbi_load(full_path.string().c_str(), &width, &height, &channels, 0);
			}
		}


		if (data.Data == nullptr)
		{
			HVE_CORE_ERROR_TAG("Texture Importer", "Could not load texture for file path: {0}", path);
			return nullptr;
		}

		data.Size = width * height * channels;

		TextureSpecification spec;
		spec.Width = width;
		spec.Height = height;

		switch (channels)
		{
			case 1:
				spec.Format = ImageFormat::R8;
				break;
			case 3:
				spec.Format = type == AssetType::CubeMap ? ImageFormat::RGB32F : ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = type == AssetType::CubeMap ? ImageFormat::RGBA32F : ImageFormat::RGBA8;
				break;
			default:
				HVE_CORE_ASSERT(false, "Image has an invalid format");
		}



		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		stbi_image_free(data.Data);
		return texture;
	}
	Ref<TextureCube> TextureImporter::ImportCube(AssetHandle handle, const AssetMetadata& metadata)
	{
		Ref<Texture2D> flat_texture = Import2D(handle, metadata);

		Ref<TextureCube> cube_texture = TextureCube::Create(flat_texture, 1024); // Hard coded for now, we might change that later

		return cube_texture;
	}
	Ref<TextureCube> TextureImporter::ImportCubeWithPath(const std::string& path)
	{
		Ref<Texture2D> flat_texture = Import2DWithPath(path);

		Ref<TextureCube> cube_texture = TextureCube::Create(flat_texture, 1024); // Hard coded for now, we might change that later

		return cube_texture;
	}
}
