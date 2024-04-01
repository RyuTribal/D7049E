#pragma once
#include <glad/gl.h>

namespace Engine {
	enum class ImageFormat
	{
		None = 0,
		R8,
		RGB8,
		RGBA8,
		RGBA32F,
		DEPTH_COMPONENT
	};

	struct TextureSpecification
	{
		uint32_t Width = 1;
		uint32_t Height = 1;
		ImageFormat Format = ImageFormat::RGBA8;
		bool GenerateMips = true;
	};

	class Texture
	{
	public:
		virtual ~Texture() = default;

		virtual const TextureSpecification& GetSpecification() const = 0;

		virtual uint32_t GetWidth() const = 0;
		virtual uint32_t GetHeight() const = 0;
		virtual uint32_t GetRendererID() const = 0;

		virtual const std::string& GetPath() const = 0;

		virtual void SetData(void* data, uint32_t size) = 0;

		virtual void Bind(uint32_t slot = 0) const = 0;

		virtual bool IsLoaded() const = 0;

		virtual bool operator==(const Texture& other) const = 0;
	};

	class Texture2D : public Texture {
	public:
		static Ref<Texture2D> Create(const TextureSpecification& specification) {
			return CreateRef<Texture2D>(specification);
		}
		static Ref<Texture2D> Create(const std::string& path) {
			return CreateRef<Texture2D>(path);
		}

		Texture2D(const TextureSpecification& specification);
		Texture2D(const std::string& path);
		~Texture2D();

		const TextureSpecification& GetSpecification() const { return m_Specification; }

		uint32_t GetWidth() const { return m_Width; }
		uint32_t GetHeight() const { return m_Height; }
		uint32_t GetRendererID() const { return m_RendererID; }

		const std::string& GetPath() const { return m_Path; }

		void SetData(void* data, uint32_t size);

		void Bind(uint32_t slot = 0) const;

		bool IsLoaded() const { return m_IsLoaded; }

		bool operator==(const Texture& other) const
		{
			return m_RendererID == other.GetRendererID();
		}
	private:
		TextureSpecification m_Specification;

		std::string m_Path;
		bool m_IsLoaded = false;
		uint32_t m_Width, m_Height;
		uint32_t m_RendererID;
		GLenum m_InternalFormat, m_DataFormat;
	};
}