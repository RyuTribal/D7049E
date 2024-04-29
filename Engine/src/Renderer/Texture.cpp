#include "pch.h"
#include "Texture.h"

namespace Engine {
	namespace Utils {

		static GLenum HeliosImageFormatToGLDataFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB;
			case ImageFormat::RGBA8: return GL_RGBA;
			}

			HVE_CORE_ASSERT(false);
			return 0;
		}

		static GLenum HeliosImageFormatToGLInternalFormat(ImageFormat format)
		{
			switch (format)
			{
			case ImageFormat::RGB8:  return GL_RGB8;
			case ImageFormat::RGBA8: return GL_RGBA8;
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
		uint32_t bpp = m_DataFormat == GL_RGBA ? 4 : 3;
		HVE_CORE_ASSERT(data.Size == m_Width * m_Height * bpp, "Data must be entire texture!");
		m_IsLoaded = true;
		glTextureSubImage2D(m_RendererID, 0, 0, 0, m_Width, m_Height, m_DataFormat, GL_UNSIGNED_BYTE, data.Data);
	}

	void Texture2D::Bind(uint32_t slot) const
	{
		glBindTextureUnit(slot, m_RendererID);
	}
}
