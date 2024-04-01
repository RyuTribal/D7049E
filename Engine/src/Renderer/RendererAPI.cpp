#include "pch.h"
#include "RendererAPI.h"
#include <glad/gl.h>

namespace Engine {

	namespace Util {
		static GLuint HeliosToNativeTextureUnit(TextureUnits unit) {
			switch (unit) {
			case TEXTURE0:		return GL_TEXTURE0;
			case TEXTURE1:		return GL_TEXTURE1;
			case TEXTURE2:		return GL_TEXTURE2;
			case TEXTURE3:		return GL_TEXTURE3;
			case TEXTURE4:		return GL_TEXTURE4;
			case TEXTURE5:		return GL_TEXTURE5;
			case TEXTURE6:		return GL_TEXTURE6;
			case TEXTURE7:		return GL_TEXTURE7;
			case TEXTURE8:		return GL_TEXTURE8;
			case TEXTURE9:		return GL_TEXTURE9;
			case TEXTURE10:		return GL_TEXTURE10;
			case TEXTURE11:		return GL_TEXTURE11;
			case TEXTURE12:		return GL_TEXTURE12;
			case TEXTURE13:		return GL_TEXTURE13;
			case TEXTURE14:		return GL_TEXTURE14;
			case TEXTURE15:		return GL_TEXTURE15;
			case TEXTURE16:		return GL_TEXTURE16;
			case TEXTURE17:		return GL_TEXTURE17;
			case TEXTURE18:		return GL_TEXTURE18;
			case TEXTURE19:		return GL_TEXTURE19;
			case TEXTURE20:		return GL_TEXTURE20;
			case TEXTURE21:		return GL_TEXTURE21;
			case TEXTURE22:		return GL_TEXTURE22;
			case TEXTURE23:		return GL_TEXTURE23;
			case TEXTURE24:		return GL_TEXTURE24;
			case TEXTURE25:		return GL_TEXTURE25;
			case TEXTURE26:		return GL_TEXTURE26;
			case TEXTURE27:		return GL_TEXTURE27;
			case TEXTURE28:		return GL_TEXTURE28;
			case TEXTURE29:		return GL_TEXTURE29;
			case TEXTURE30:		return GL_TEXTURE30;
			case TEXTURE31:		return GL_TEXTURE31;
			}
		}


		static TextureUnits NativeToHeliosTextureUnit(GLuint unit) {
			switch (unit) {
			case GL_TEXTURE0:		return TEXTURE0;
			case GL_TEXTURE1:		return TEXTURE1;
			case GL_TEXTURE2:		return TEXTURE2;
			case GL_TEXTURE3:		return TEXTURE3;
			case GL_TEXTURE4:		return TEXTURE4;
			case GL_TEXTURE5:		return TEXTURE5;
			case GL_TEXTURE6:		return TEXTURE6;
			case GL_TEXTURE7:		return TEXTURE7;
			case GL_TEXTURE8:		return TEXTURE8;
			case GL_TEXTURE9:		return TEXTURE9;
			case GL_TEXTURE10:		return TEXTURE10;
			case GL_TEXTURE11:		return TEXTURE11;
			case GL_TEXTURE12:		return TEXTURE12;
			case GL_TEXTURE13:		return TEXTURE13;
			case GL_TEXTURE14:		return TEXTURE14;
			case GL_TEXTURE15:		return TEXTURE15;
			case GL_TEXTURE16:		return TEXTURE16;
			case GL_TEXTURE17:		return TEXTURE17;
			case GL_TEXTURE18:		return TEXTURE18;
			case GL_TEXTURE19:		return TEXTURE19;
			case GL_TEXTURE20:		return TEXTURE20;
			case GL_TEXTURE21:		return TEXTURE21;
			case GL_TEXTURE22:		return TEXTURE22;
			case GL_TEXTURE23:		return TEXTURE23;
			case GL_TEXTURE24:		return TEXTURE24;
			case GL_TEXTURE25:		return TEXTURE25;
			case GL_TEXTURE26:		return TEXTURE26;
			case GL_TEXTURE27:		return TEXTURE27;
			case GL_TEXTURE28:		return TEXTURE28;
			case GL_TEXTURE29:		return TEXTURE29;
			case GL_TEXTURE30:		return TEXTURE30;
			case GL_TEXTURE31:		return TEXTURE31;
			}
		}
	}

	void MessageCallback(
		unsigned source,
		unsigned type,
		unsigned id,
		unsigned severity,
		int length,
		const char* message,
		const void* userParam)
	{
		switch (severity)
		{
		case GL_DEBUG_SEVERITY_HIGH:         HVE_CORE_FATAL(message); return;
		case GL_DEBUG_SEVERITY_MEDIUM:       HVE_CORE_ERROR(message);  return;
		case GL_DEBUG_SEVERITY_LOW:          HVE_CORE_WARN(message);  return;
		case GL_DEBUG_SEVERITY_NOTIFICATION: HVE_CORE_TRACE(message);  return;
		}

		HVE_CORE_ASSERT(false, "Unknown severity level!");
	}

	void RendererAPI::Init()
	{

#ifdef DEBUG
		glEnable(GL_DEBUG_OUTPUT);
		glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
		glDebugMessageCallback(MessageCallback, nullptr);

		glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, NULL, GL_FALSE);
#endif

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
	}

	void RendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
	{
		glViewport(x, y, width, height);
	}

	void RendererAPI::SetClearColor(const glm::vec4& color)
	{
		glClearColor(color.r, color.g, color.b, color.a);
	}

	void RendererAPI::ClearAll()
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	}

	void RendererAPI::ClearDepth()
	{
		glClear(GL_DEPTH_BUFFER_BIT);
	}

	void RendererAPI::ClearColor()
	{
		glClear(GL_COLOR_BUFFER_BIT);
	}

	void RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		vertexArray->Unbind();
	}

	void RendererAPI::DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount)
	{
		vertexArray->Bind();
		glDrawArrays(GL_LINES, 0, vertexCount);
		vertexArray->Unbind();
	}

	void RendererAPI::UseShaderProgram(uint32_t id)
	{
		glUseProgram(id);
	}

	void RendererAPI::DispatchCompute(uint32_t x, uint32_t y, uint32_t z)
	{
		glDispatchCompute(x, y, z);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void RendererAPI::ActivateTextureUnit(TextureUnits unit)
	{
		glActiveTexture(Util::HeliosToNativeTextureUnit(unit));
	}

	void RendererAPI::BindTexture(uint32_t texture_id)
	{
		glBindTexture(GL_TEXTURE_2D, texture_id);
	}

	void RendererAPI::UnBindTexture(uint32_t texture_id)
	{
		glBindTexture(GL_TEXTURE_2D, 0);
	}

	void RendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}


}