#pragma once
#include <glad/gl.h>


namespace Engine {
	class Mesh {
	public:
		Mesh() = default;
		Mesh(std::vector<Vertex>& vertices, std::vector<int> indices);
		~Mesh();

		void SetData(std::vector<Vertex>& vertices, std::vector<int> indices);
		void SetTransform(glm::mat4 transform) { m_Transform = transform; }
		void BindData();

		std::vector<Ref<Vertex>>& GetVertices() { return m_Vertices; }
		std::vector<int>& GetIndices() { return m_Indices; }
		glm::mat4& GetTransform() { return m_Transform; }

		GLuint& GetVertexBuffer() { return m_VertexBufferObject; }
		GLuint& GetIndexBuffer() { return m_IndexBufferObject; }
		GLuint& GetVertexArrayObject() { return m_VertexArrayObject; }

	private:
		std::vector<Ref<Vertex>> m_Vertices = {};
		std::vector<int> m_Indices = {};
		glm::mat4 m_Transform = glm::mat4(0.f);
		GLuint m_VertexBufferObject;
		GLuint m_IndexBufferObject;
		GLuint m_VertexArrayObject;
	};
}