#pragma once
#include "VertexArray.h"

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

	class RendererAPI {
	public:
		void Init();
		void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height);

		void SetClearColor(const glm::vec4& color);
		void ClearAll();
		void ClearDepth();
		void ClearColor();

		void DrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		void DrawLines(const Ref<VertexArray>& vertexArray, uint32_t vertexCount);
		void DrawLine(const glm::vec3& start, const glm::vec3& end);

		void UseShaderProgram(uint32_t id);
		void DispatchCompute(uint32_t x, uint32_t y, uint32_t z);

		void ActivateTextureUnit(TextureUnits unit);
		void BindTexture(uint32_t texture_id);
		void UnBindTexture(uint32_t texture_id);
		void UnBindBuffer();
		void SetDepthWriting(bool write);

		void SetLineWidth(float width);
	};
}
