#include "pch.h"
#include "Texture.h"

namespace Engine {
	namespace Utils {

		static GLenum HeliosImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:		return GL_RED_INTEGER;
			case ImageFormat::RG8:		return GL_RG;
			case ImageFormat::RGB8:		return GL_RGB;
			case ImageFormat::RGBA8:	return GL_RGBA;
			case ImageFormat::RGB16F:	return GL_RGB;
			case ImageFormat::RGBA16F:	return GL_RGBA;
			case ImageFormat::RG32F:	return GL_RG;
			case ImageFormat::RGB32F:	return GL_RGB;
			case ImageFormat::RGBA32F:	return GL_RGBA;
			}

			HVE_CORE_ASSERT(false);
			return 0;
		}

		static GLenum HeliosImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::R8:		return GL_RED_INTEGER;
			case ImageFormat::RG8:		return GL_RG8;
			case ImageFormat::RGB8:		return GL_RGB8;
			case ImageFormat::RGBA8:	return GL_RGBA8;
			case ImageFormat::RGB16F:	return GL_RGB16F;
			case ImageFormat::RGBA16F:	return GL_RGBA16F;
			case ImageFormat::RG32F:	return GL_RG32F;
			case ImageFormat::RGB32F:	return GL_RGB32F;
			case ImageFormat::RGBA32F:	return GL_RGBA32F;
			}

			HVE_CORE_ASSERT(false);
			return 0;
		}

		static uint32_t ImageFormatToChannels(ImageFormat format)
		{
			switch (format)
			{
				case ImageFormat::R8:		return 1;
				case ImageFormat::RG8:		return 2;
				case ImageFormat::RGB8:		return 3;
				case ImageFormat::RGBA8:	return 4;
				case ImageFormat::RGB16F:	return 3;
				case ImageFormat::RGBA16F:	return 4;
				case ImageFormat::RG32F:	return 2;
				case ImageFormat::RGB32F:	return 3;
				case ImageFormat::RGBA32F:	return 4;
			}

			HVE_CORE_ASSERT(false);
			return 0;
		}
	}

	Texture2D::Texture2D(const TextureSpecification& specification, Buffer data)
		: m_Specification(specification), m_Width(m_Specification.Width), m_Height(m_Specification.Height)
	{
		

		m_InternalFormat = Utils::HeliosImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::HeliosImageFormatToGLDataFormat(m_Specification.Format);
		m_IsFloat = m_InternalFormat == GL_RGB16F || m_InternalFormat == GL_RGBA16F || m_InternalFormat == GL_RGBA32F || m_InternalFormat == GL_RGB32F || m_InternalFormat == GL_RG32F;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		if (m_Specification.Format == ImageFormat::DEPTH_COMPONENT) {
			glTextureStorage2D(m_RendererID, 1, GL_DEPTH_COMPONENT24, m_Width, m_Height);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
			GLfloat borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
			glTextureParameterfv(m_RendererID, GL_TEXTURE_BORDER_COLOR, borderColor);
		}
		else {
			glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);

			glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		if (data)
		{
			SetData(data);
		}
	}

	Texture2D::Texture2D(Ref<Texture2D> other)
		: m_Specification(other->GetSpecification()), m_Width(other->m_Width), m_Height(other->m_Height)
	{
		m_InternalFormat = Utils::HeliosImageFormatToGLInternalFormat(m_Specification.Format);
		m_DataFormat = Utils::HeliosImageFormatToGLDataFormat(m_Specification.Format);
		m_IsFloat = m_InternalFormat == GL_RGB16F || m_InternalFormat == GL_RGBA16F || m_InternalFormat == GL_RGBA32F || m_InternalFormat == GL_RGB32F || m_InternalFormat == GL_RG32F;

		glCreateTextures(GL_TEXTURE_2D, 1, &m_RendererID);
		glTextureStorage2D(m_RendererID, 1, m_InternalFormat, m_Width, m_Height);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTextureParameteri(m_RendererID, GL_TEXTURE_WRAP_T, GL_REPEAT);

		CopyTextureData(other->m_RendererID);
		m_IsLoaded = true;
	}

	void Texture2D::CopyTextureData(GLuint srcTextureID)
	{
		GLuint fbo;
		glGenFramebuffers(1, &fbo);
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, srcTextureID, 0);

		if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		{
			HVE_CORE_ERROR_TAG("Texture", "Framebuffer is not complete!");
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
			glDeleteFramebuffers(1, &fbo);
			return;
		}

		glBindTexture(GL_TEXTURE_2D, m_RendererID);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, m_Width, m_Height);

		// Clean up
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDeleteFramebuffers(1, &fbo);
	}

	Texture2D::~Texture2D()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void Texture2D::SetData(Buffer data)
	{
		//uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		m_Channels = Utils::ImageFormatToChannels(m_Specification.Format);
		HVE_CORE_ASSERT(data.Size == m_Width * m_Height * m_Channels, "Data must be entire texture!");
		m_IsLoaded = true;

		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, m_IsFloat ? GL_FLOAT : GL_UNSIGNED_BYTE, data.Data);
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}


	TextureCube::TextureCube(Ref<Texture2D> map_texture, uint32_t size, ImageFormat format) : m_FlattenedTexture(map_texture), m_Size(size)
	{
		bool is_proper_format = format == ImageFormat::RGB16F || format == ImageFormat::RGBA16F || format == ImageFormat::RGB32F || format == ImageFormat::RGBA32F;
		HVE_CORE_ASSERT(is_proper_format, "Cubemap pixels must be of type float to capture those sweet sweet details");

		glGenTextures(1, &m_RendererID);
		glBindTexture(GL_TEXTURE_CUBE_MAP, m_RendererID);
		for (unsigned int i = 0; i < 6; ++i)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, Utils::HeliosImageFormatToGLInternalFormat(format), m_Size, m_Size, 0,
						 GL_RGB, GL_FLOAT, nullptr);
		}

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}

	TextureCube::~TextureCube()
	{
		glDeleteTextures(1, &m_RendererID);
	}

	void TextureCube::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
	void TextureCube::SetData(Buffer data)
	{
		m_FlattenedTexture->SetData(data);
	}
	void TextureCube::GenerateMipMap()
	{
		Bind();
		glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
	}
}
