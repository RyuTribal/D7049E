#pragma once
#include <glad/gl.h>
#include "VertexArray.h"


namespace Engine {
	class Mesh {
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t> indices);
		~Mesh();

		void Mesh::Bind() const;
		void Mesh::Unbind() const;

		void SetTransform(glm::mat4 transform) { m_Transform = transform; }

		glm::mat4& GetTransform() { return m_Transform; }

		Ref<VertexArray> GetVertexArray() { return m_VertexArray; }

	private:
		Ref<VertexArray> m_VertexArray;
		glm::mat4 m_Transform = glm::mat4(0.f);
	};
}