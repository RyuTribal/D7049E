#pragma once
#include <glad/gl.h>
#include "VertexArray.h"


namespace Engine {

	struct VertexData
	{
		glm::vec3 coordinates = { 0.f, 0.f, 0.f };
		glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
		glm::vec2 texture_coordinates = { 1.f, 1.f };
		glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
		uint32_t entity_id = -1;
	};

	class Mesh {
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t> indices);
		~Mesh();

		void Bind() const;
		void Unbind() const;

		void SetTransform(glm::mat4 transform) { m_Transform = transform; }

		glm::mat4& GetTransform() { return m_Transform; }

		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

		int Size() { return m_VertexCount; }

	private:
		Ref<VertexArray> m_VertexArray;
		glm::mat4 m_Transform = glm::mat4(0.f);
		int m_VertexCount = 0;
	};
}
