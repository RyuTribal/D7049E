#include "pch.h"
#include "RendererAPI.h"
#include <glad/gl.h>
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

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

		static GLuint HeliosToNativeDepthFunction(DepthFunction func)
		{
			switch (func)
			{
				case LEqual:		return GL_LEQUAL;
				case Less:			return GL_LESS;
			}
		}

		static DepthFunction NativeToHeliosDepthFunction(GLuint func)
		{
			switch (func)
			{
				case GL_LEQUAL:		return LEqual;
				case GL_LESS:		return Less;
			}
		}

		static GLuint HeliosToNativeCullOption(CullOption option)
		{
			switch (option)
			{
				case FRONT:			return GL_FRONT;
				case BACK:			return GL_BACK;
			}
		}

		static CullOption NativeToHeliosCullOption(GLuint func)
		{
			switch (func)
			{
				case GL_FRONT:		return FRONT;
				case GL_BACK:		return BACK;
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
			case GL_DEBUG_SEVERITY_HIGH:		 HVE_CORE_FATAL(message); return;
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

		glEnable(GL_MULTISAMPLE);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LINE_SMOOTH);
		glDepthMask(GL_TRUE);
	}

	uint32_t RendererAPI::GetCurrentShaderProgram()
	{
		GLint prog = 0;
		glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
		return prog;
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

	void RendererAPI::SetCull(CullOption option)
	{
		glCullFace(Util::HeliosToNativeCullOption(option));
	}

	void RendererAPI::SetDepthFunction(DepthFunction func)
	{
		glDepthFunc(Util::HeliosToNativeDepthFunction(func));
	}

	void RendererAPI::DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount)
	{
		vertexArray->Bind();
		uint32_t count = indexCount ? indexCount : vertexArray->GetIndexBuffer()->GetCount();
		glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
		vertexArray->Unbind();
	}

	void RendererAPI::DrawInstancedLines(std::vector<Line>& lines)
	{
		if (lines.empty()) return;

		std::vector<float> vertices;
		std::vector<glm::vec4> colors;
		std::vector<glm::mat4> transforms;

		for (const Line& line : lines)
		{
			vertices.push_back(line.Start.x);
			vertices.push_back(line.Start.y);
			vertices.push_back(line.Start.z);

			vertices.push_back(line.End.x);
			vertices.push_back(line.End.y);
			vertices.push_back(line.End.z);

			colors.push_back(line.Color);
			colors.push_back(line.Color); // Color for both points

			transforms.push_back(line.Transform); // Add line's transformation matrix
		}

		GLuint vao, vbo, cbo, tbo;
		glGenVertexArrays(1, &vao);
		glGenBuffers(1, &vbo);
		glGenBuffers(1, &cbo);
		glGenBuffers(1, &tbo);

		// Vertex Buffer Setup
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

		// Color Buffer Setup
		glBindBuffer(GL_ARRAY_BUFFER, cbo);
		glBufferData(GL_ARRAY_BUFFER, colors.size() * sizeof(glm::vec4), colors.data(), GL_STATIC_DRAW);

		// Transformation Buffer Setup
		glBindBuffer(GL_ARRAY_BUFFER, tbo);
		glBufferData(GL_ARRAY_BUFFER, transforms.size() * sizeof(glm::mat4), transforms.data(), GL_STATIC_DRAW);

		// VAO Setup
		glBindVertexArray(vao);

		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, cbo);
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ARRAY_BUFFER, tbo);

		for (int i = 0; i < 4; ++i)
		{
			glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * 4 * sizeof(float)));
			glEnableVertexAttribArray(2 + i);
			glVertexAttribDivisor(2 + i, 1); // Per-instance divisor
		}

		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(vao);

		// Draw all lines in one call
		glDrawArraysInstanced(GL_LINES, 0, vertices.size() / 3, lines.size());

		glBindVertexArray(0);
		glDeleteBuffers(1, &vbo);
		glDeleteBuffers(1, &cbo);
		glDeleteBuffers(1, &tbo);
		glDeleteVertexArrays(1, &vao);
	}

	void RendererAPI::DrawLine(const glm::vec3& start, const glm::vec3& end)
	{
		float vertices[6] = {
		start.x, start.y, start.z,
		end.x, end.y, end.z
		};

		Ref<VertexArray> VAO = VertexArray::Create();
		uint32_t size = 6 * sizeof(float);
		auto vertexBuffer = VertexBuffer::Create(size);
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_position" }});
		vertexBuffer->SetData(&vertices, size);
		VAO->AddVertexBuffer(vertexBuffer);

		VAO->Bind();

		glDrawArrays(GL_LINES, 0, 2);
		VAO->Unbind();
	}

	void RendererAPI::DrawQuad()
	{

		float vertices[] = {
			// Positions         // Texture Coords (s, t)
			-1.0f, -1.0f, 0.0f,  0.0f, 0.0f, // bottom left corner
			-1.0f,  1.0f, 0.0f,  0.0f, 1.0f, // top left corner
			 1.0f,  1.0f, 0.0f,  1.0f, 1.0f, // top right corner
			 1.0f, -1.0f, 0.0f,  1.0f, 0.0f  // bottom right corner
		};

		uint32_t indices[] = {
			0, 1, 2, // first triangle (bottom left - top left - top right)
			0, 2, 3  // second triangle (bottom left - top right - bottom right)
		};

		unsigned int vertexSize = sizeof(vertices);
		unsigned int indexCount = sizeof(indices) / sizeof(uint32_t);

		Ref<VertexArray> VAO = VertexArray::Create();
		auto vertexBuffer = VertexBuffer::Create(vertices, vertexSize);
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_position" },
			{ ShaderDataType::Float2, "a_texture_coords" } });
		vertexBuffer->SetData(&vertices, vertexSize);

		auto indexBuffer = IndexBuffer::Create(indices, indexCount);
		VAO->AddVertexBuffer(vertexBuffer);
		VAO->SetIndexBuffer(indexBuffer);

		DrawIndexed(VAO);
	}



	void RendererAPI::DrawCube()
	{
		float vertices[] = {
			-1.0f, -1.0f, -1.0f, // 0
			 1.0f, -1.0f, -1.0f, // 1
			 1.0f,  1.0f, -1.0f, // 2
			-1.0f,  1.0f, -1.0f, // 3
			-1.0f, -1.0f,  1.0f, // 4
			 1.0f, -1.0f,  1.0f, // 5
			 1.0f,  1.0f,  1.0f, // 6
			-1.0f,  1.0f,  1.0f  // 7
		};

		// Indices for the cube (12 triangles, two for each face)
		uint32_t indices[] = {
			// Back face
			0, 1, 2, 2, 3, 0,
			// Front face
			4, 7, 6, 6, 5, 4,
			// Left face
			3, 2, 6, 6, 7, 3,
			// Right face
			1, 0, 4, 4, 5, 1,
			// Top face
			2, 1, 5, 5, 6, 2,
			// Bottom face
			0, 3, 7, 7, 4, 0
		};

		unsigned int vertexSize = sizeof(vertices);
		unsigned int indexCount = sizeof(indices) / sizeof(uint32_t);

		Ref<VertexArray> VAO = VertexArray::Create();
		auto vertexBuffer = VertexBuffer::Create(vertices, vertexSize);
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_position" } });
		vertexBuffer->SetData(&vertices, vertexSize);

		auto indexBuffer = IndexBuffer::Create(indices, indexCount);
		VAO->AddVertexBuffer(vertexBuffer);
		VAO->SetIndexBuffer(indexBuffer);

		DrawIndexed(VAO);
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

	// Deprecated
	void RendererAPI::ActivateTextureUnit(TextureUnits unit)
	{
		glActiveTexture(Util::HeliosToNativeTextureUnit(unit));
	}

	void RendererAPI::BindTexture(uint32_t texture_id, uint32_t slot)
	{
		glBindTextureUnit(slot, texture_id);
	}

	void RendererAPI::UnBindBuffer()
	{
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void RendererAPI::SetDepthWriting(bool write)
	{
		glDepthMask(write ? GL_TRUE : GL_FALSE);
	}

	void RendererAPI::SetLineWidth(float width)
	{
		glLineWidth(width);
	}


}
