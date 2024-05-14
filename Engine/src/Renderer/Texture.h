#pragma once
#include <glad/gl.h>

#include "Assets/Asset.h"
#include "Core/Buffer.h"

namespace Engine {
	enum class ImageFormat
	{
		None = 0,
		R8,
		RG8,
		RGB8,
		RGBA8,
		RGB16F,
		RGBA16F,
		RG32F,
		RGB32F,
		RGBA32F,
		DEPTH_COMPONENT
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		uint32_t Size = 0;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = true;
	};

	class Texture : public Asset
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual void SetData(Buffer data) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification, Buffer data) {
			return CreateRef<Texture2D>(specification, data);
		}

		Texture2D(const TextureSpecification& specification, Buffer data);
		Texture2D(Ref<Texture2D> other);
		~Texture2D();

		const TextureSpecification& GetSpecification() const { return m_Specification; }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		uint32_t GetRendererID() const { return m_RendererID; }
		void CopyTextureData(GLuint srcTextureID);

		void SetData(Buffer data);

		void Bind(uint32_t slot = 0) const;

		bool IsLoaded() const 
		{ 
			if (!m_IsLoaded || m_RendererID == 0)
			{
				return false;
			}
			return glIsTexture(m_RendererID) == GL_TRUE;
		}

		bool operator==(const Texture& other) const
		{
			return m_RendererID == other.GetRendererID();
		}


		static AssetType GetStaticType() { return AssetType::Texture; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

	private:
		TextureSpecification m_Specification;
		bool m_IsLoaded = false;
		bool m_IsFloat = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;

		uint32_t m_Channels = 4;
	};


	class TextureCube : public Texture
	{
	public:
		static Ref<TextureCube> Create(Ref<Texture2D> map_texture, uint32_t size)
		{
			return CreateRef<TextureCube>(map_texture, size, map_texture->GetSpecification().Format);
		}

		static Ref<TextureCube> Create(Ref<Texture2D> map_texture, uint32_t size, ImageFormat format)
		{
			return CreateRef<TextureCube>(map_texture, size, format);
		}

		TextureCube(Ref<Texture2D> map_texture, uint32_t size, ImageFormat format);
		~TextureCube();

		const TextureSpecification& GetSpecification() const { return m_FlattenedTexture->GetSpecification(); }

		uint32_t GetWidth() const { return m_Size; }
		uint32_t GetHeight() const { return m_Size; }
		uint32_t GetRendererID() const { return m_RendererID; }

		void Bind(uint32_t slot = 0) const;

		Ref<Texture2D> GetFlatTexture() { return m_FlattenedTexture; }

		void SetData(Buffer data);

		void GenerateMipMap();


		bool IsLoaded() const
		{
			if (!m_IsLoaded || m_RendererID == 0)
			{
				return false;
			}
			return glIsTexture(m_RendererID) == GL_TRUE;
		}

		bool operator==(const Texture& other) const
		{
			return m_RendererID == other.GetRendererID();
		}


		static AssetType GetStaticType() { return AssetType::CubeMap; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

	private:
		bool m_IsLoaded = false;
		uint32_t m_Size;
		uint32_t m_RendererID;
		Ref<Texture2D> m_FlattenedTexture;
	};


}
