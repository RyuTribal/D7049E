#pragma once
#include "VertexArray.h"
#include "ShaderProgram.h"

namespace Engine {
	enum TextureUnits {
		TEXTURE0,
		TEXTURE1,
		TEXTURE2,
		TEXTURE3,
		TEXTURE4,
		TEXTURE5,
		TEXTURE6,
		TEXTURE7,
		TEXTURE8,
		TEXTURE9,
		TEXTURE10,
		TEXTURE11,
		TEXTURE12,
		TEXTURE13,
		TEXTURE14,
		TEXTURE15,
		TEXTURE16,
		TEXTURE17,
		TEXTURE18,
		TEXTURE19,
		TEXTURE20,
		TEXTURE21,
		TEXTURE22,
		TEXTURE23,
		TEXTURE24,
		TEXTURE25,
		TEXTURE26,
		TEXTURE27,
		TEXTURE28,
		TEXTURE29,
		TEXTURE30,
		TEXTURE31
		
	};

	enum CullOption
	{
		BACK,
		FRONT
	};

	enum DepthFunction
	{
		LEqual,
		Less
	};

	struct Line
	{
		glm::vec3 Start;
		glm::vec3 End;
		glm::vec4 Color;
		glm::mat4 Transform;
	};

	class RendererAPI {
	public:
		void Init();

		uint32_t GetCurrentShaderProgram();

		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		void SetClearColor(const glm::vec4& color);
		void ClearAll();
		void ClearDepth();
		void ClearColor();
		void SetCull(CullOption option);

		void SetDepthFunction(DepthFunction func);

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		void DrawInstancedLines(std::vector<Line>& lines);
		void DrawLine(const glm::vec3& start, const glm::vec3& end);
		void DrawQuad();
		void DrawCube();

		void UseShaderProgram(uint32_t id);
		void DispatchCompute(uint32_t x, uint32_t y, uint32_t z);

		void ActivateTextureUnit(TextureUnits unit);
		void BindTexture(uint32_t texture_id, uint32_t slot = 0);
		void UnBindBuffer();
		void SetDepthWriting(bool write);

		void SetLineWidth(float width);
	};
}
