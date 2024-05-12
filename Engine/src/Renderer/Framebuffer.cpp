#include "pch.h"
#include "Framebuffer.h"
#include "glad/gl.h"


namespace Engine{
	static const uint32_t s_MaxFramebufferSize = 8192;

	namespace Utils {

		static GLenum TextureTarget(bool multisampled, bool multidimensional)
		{
			if (multidimensional)
			{
				return multisampled ? GL_TEXTURE_2D_MULTISAMPLE_ARRAY : GL_TEXTURE_2D_ARRAY;
			}
			return multisampled ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D;
		}

		static void CreateTextures(bool multisampled, bool multidimensional, uint32_t* outID, uint32_t count)
		{
			glCreateTextures(TextureTarget(multisampled, multidimensional), count, outID);
		}

		static void BindTexture(bool multisampled, bool multidimensional, uint32_t id)
		{
			glBindTexture(TextureTarget(multisampled, multidimensional), id);
		}

		static void AttachColorTexture(uint32_t id, int samples, GLenum internalFormat, GLenum format, uint32_t width, uint32_t height, int index, int array_size)
		{
			bool multisampled = samples > 1;
			bool multidimensional = array_size > 1;
			if (multisampled)
			{
				glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, internalFormat, width, height, GL_FALSE);
			}
			else if (internalFormat == GL_RGBA16F || internalFormat == GL_RGB16F || internalFormat == GL_RGBA32F || internalFormat == GL_RGB32F)
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_FLOAT, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
			else
			{
				glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, GL_UNSIGNED_BYTE, nullptr);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}

			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + index, TextureTarget(multisampled, multidimensional), id, 0);
		}

		static void AttachDepthTexture(uint32_t id, int samples, GLenum format, GLenum attachmentType, uint32_t width, uint32_t height, int array_size)
		{
			bool multisampled = samples > 1;
			bool multidimensional = array_size > 1;
			if (multidimensional)
			{
				if (multisampled)
				{
					glTexImage3DMultisample(GL_TEXTURE_2D_MULTISAMPLE_ARRAY, samples, format, width, height, array_size, GL_FALSE);
				}
				else
				{
					glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, format, width, height, array_size);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				glFramebufferTexture(GL_FRAMEBUFFER, attachmentType, id, 0);
			}
			else
			{
				if (multisampled)
				{
					glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, format, width, height, GL_FALSE);
				}
				else
				{
					glTexStorage2D(GL_TEXTURE_2D, 1, format, width, height);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
				glFramebufferTexture2D(GL_FRAMEBUFFER, attachmentType, TextureTarget(multisampled, multidimensional), id, 0);
			}
		}

		static bool IsDepthFormat(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:  return true;
			}

			return false;
		}

		static GLenum HeliosFBTextureFormatToGL(FramebufferTextureFormat format)
		{
			switch (format)
			{
			case FramebufferTextureFormat::RGBA8:       return GL_RGBA8;
			case FramebufferTextureFormat::RG16F:		return GL_RG16F;
			case FramebufferTextureFormat::RGBA16F:		return GL_RGBA16F;
			case FramebufferTextureFormat::RG32F:		return GL_RG32F;
			case FramebufferTextureFormat::RGBA32F:		return GL_RGBA32F;
			case FramebufferTextureFormat::RED_INTEGER: return GL_RED_INTEGER;
			}

			HVE_CORE_ASSERT(false);
			return 0;
		}

		static GLenum HeliosDownSamplingFormatToGL(FramebufferSamplingFormat format)
		{

			switch (format)
			{
				case FramebufferSamplingFormat::Linear:			return GL_LINEAR;
				case FramebufferSamplingFormat::Nearest:		return GL_NEAREST;
			}

			HVE_CORE_ASSERT(false);
			return 0;
			 
		}

	}

	Framebuffer::Framebuffer(const FramebufferSpecification& spec)
		: m_Specification(spec)
	{
		for (auto spec : m_Specification.Attachments.Attachments)
		{
			if (!Utils::IsDepthFormat(spec.TextureFormat))
				m_ColorAttachmentSpecifications.emplace_back(spec);
			else
				m_DepthAttachmentSpecification = spec;
		}

		Invalidate();
	}

	Framebuffer::~Framebuffer()
	{
		glDeleteFramebuffers(1, &m_RendererID);
		glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
		glDeleteTextures(1, &m_DepthAttachment);
	}

	void Framebuffer::Invalidate()
	{
		if (m_RendererID)
		{
			glDeleteFramebuffers(1, &m_RendererID);
			glDeleteTextures(m_ColorAttachments.size(), m_ColorAttachments.data());
			glDeleteTextures(1, &m_DepthAttachment);

			m_ColorAttachments.clear();
			m_DepthAttachment = 0;
		}

		glCreateFramebuffers(1, &m_RendererID);
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);

		bool multisample = m_Specification.Samples > 1;
		bool multidimensional = m_Specification.ArraySize > 1;

		// Attachments
		if (m_ColorAttachmentSpecifications.size())
		{
			m_ColorAttachments.resize(m_ColorAttachmentSpecifications.size());
			Utils::CreateTextures(multisample, multidimensional, m_ColorAttachments.data(), m_ColorAttachments.size());

			for (size_t i = 0; i < m_ColorAttachments.size(); i++)
			{
				Utils::BindTexture(multisample, multidimensional, m_ColorAttachments[i]);
				switch (m_ColorAttachmentSpecifications[i].TextureFormat)
				{
				case FramebufferTextureFormat::RGBA8:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA8, GL_RGBA, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				case FramebufferTextureFormat::RG16F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RG16F, GL_RG, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				case FramebufferTextureFormat::RGBA16F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA16F, GL_RGBA, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				case FramebufferTextureFormat::RG32F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RG32F, GL_RG, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				case FramebufferTextureFormat::RGBA32F:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_RGBA32F, GL_RGBA, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				case FramebufferTextureFormat::RED_INTEGER:
					Utils::AttachColorTexture(m_ColorAttachments[i], m_Specification.Samples, GL_R32I, GL_RED_INTEGER, m_Specification.Width, m_Specification.Height, i, m_Specification.ArraySize);
					break;
				}
			}
		}

		if (m_DepthAttachmentSpecification.TextureFormat != FramebufferTextureFormat::None)
		{
			Utils::CreateTextures(multisample, multidimensional, &m_DepthAttachment, 1);
			Utils::BindTexture(multisample, multidimensional, m_DepthAttachment);
			switch (m_DepthAttachmentSpecification.TextureFormat)
			{
			case FramebufferTextureFormat::DEPTH24STENCIL8:
				Utils::AttachDepthTexture(m_DepthAttachment, m_Specification.Samples, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL_ATTACHMENT, m_Specification.Width, m_Specification.Height, m_Specification.ArraySize);
				break;
			}
		}

		if (m_ColorAttachments.size() > 1)
		{
			HVE_CORE_ASSERT(m_ColorAttachments.size() <= 4);
			GLenum buffers[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3};
			glDrawBuffers(m_ColorAttachments.size(), buffers);
		}
		else if (m_ColorAttachments.empty())
		{
			// Only depth-pass
			glDrawBuffer(GL_NONE);
		}

		HVE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer is incomplete!");

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Bind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, m_RendererID);
		glViewport(0, 0, m_Specification.Width, m_Specification.Height);
	}

	void Framebuffer::Unbind()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Framebuffer::Resize(uint32_t width, uint32_t height)
	{
		if (width == 0 || height == 0 || width > s_MaxFramebufferSize || height > s_MaxFramebufferSize)
		{
			HVE_CORE_WARN_TAG("FrameBuffer", "Attempted to rezize framebuffer to {0}, {1}", width, height);
			return;
		}
		m_Specification.Width = width;
		m_Specification.Height = height;

		Invalidate();
	}

	int Framebuffer::ReadPixel(uint32_t attachmentIndex, int x, int y)
	{
		HVE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		glReadBuffer(GL_COLOR_ATTACHMENT0 + attachmentIndex);
		int pixelData;
		glReadPixels(x, y, 1, 1, GL_RED_INTEGER, GL_INT, &pixelData);
		return pixelData;

	}

	void Framebuffer::ClearAttachment(uint32_t attachmentIndex, int value)
	{
		HVE_CORE_ASSERT(attachmentIndex < m_ColorAttachments.size());

		auto& spec = m_ColorAttachmentSpecifications[attachmentIndex];
		glClearTexImage(m_ColorAttachments[attachmentIndex], 0,
			Utils::HeliosFBTextureFormatToGL(spec.TextureFormat), GL_INT, &value);
	}



	void Framebuffer::SetTexture(uint32_t index, uint32_t texture_id, uint32_t mip_level)
	{
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, index, texture_id, mip_level);

		HVE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete after setting texture");
	}

	void Framebuffer::CopyFramebufferContent(Ref<Framebuffer> other_buffer, FramebufferSamplingFormat sampling)
	{
		glBindFramebuffer(GL_READ_FRAMEBUFFER, other_buffer->GetRendererID());
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_RendererID);

		int src_width = other_buffer->GetSpecification().Width;
		int src_height = other_buffer->GetSpecification().Height;

		glBlitFramebuffer(0, 0, src_width, src_height, 0, 0, m_Specification.Width, m_Specification.Height, GL_COLOR_BUFFER_BIT, Utils::HeliosDownSamplingFormatToGL(sampling));
		
		HVE_CORE_ASSERT(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE, "Framebuffer incomplete during content copying.");
	}
}
