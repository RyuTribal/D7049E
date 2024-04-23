#include "pch.h"
#include "TextureImporter.h"
#include "Core/Buffer.h"
#include "Project/Project.h"

#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Engine {
	Ref<Texture2D> TextureImporter::Import(AssetHandle handle, const AssetMetadata& metadata)
	{
		HVE_PROFILE_FUNC();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = nullptr;
		{
			HVE_PROFILE_SCOPE("stbi_load - TextureImporter::Import");
			std::filesystem::path full_path = Project::GetFullFilePath(metadata.FilePath.string().c_str());
			data.Data = stbi_load(full_path.string().c_str(), &width, &height, &channels, 0);
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
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
			default:
				HVE_CORE_ASSERT(false, "Image has an invalid format");
		}

		
		
		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		stbi_image_free(data.Data);
		return texture;
	}
	Ref<Texture2D> TextureImporter::ImportWithPath(const std::string& path)
	{
		HVE_PROFILE_FUNC();
		int width, height, channels;
		stbi_set_flip_vertically_on_load(1);
		Buffer data = nullptr;
		{
			HVE_PROFILE_SCOPE("stbi_load - TextureImporter::Import");
			data.Data = stbi_load(path.c_str(), &width, &height, &channels, 0);
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
			case 3:
				spec.Format = ImageFormat::RGB8;
				break;
			case 4:
				spec.Format = ImageFormat::RGBA8;
				break;
			default:
				HVE_CORE_ASSERT(false, "Image has an invalid format");
		}



		Ref<Texture2D> texture = Texture2D::Create(spec, data);
		stbi_image_free(data.Data);
		return texture;
	}
}
