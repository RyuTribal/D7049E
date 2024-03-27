#include "pch.h"
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<int> indices)
	{
		SetData(vertices, indices);
	}
	Mesh::~Mesh()
	{
		glDeleteVertexArrays(1, &m_VertexArrayObject);
		glDeleteBuffers(1, &m_VertexBufferObject);
		glDeleteBuffers(1, &m_IndexBufferObject);
	}
	void Mesh::SetData(std::vector<Vertex>& vertices, std::vector<int> indices)
	{
		for (Vertex& vertex : vertices) {
			Ref<Vertex> pointer_vertex = CreateRef<Vertex>(vertex.coordinates, vertex.color, vertex.texture_coordinates, vertex.normal);
			m_Vertices.push_back(pointer_vertex);
		}


		for (int index : indices) {
			m_Indices.push_back(index);
		}

		glCreateVertexArrays(1, &m_VertexArrayObject);
		glCreateBuffers(1, &m_VertexBufferObject);
		glCreateBuffers(1, &m_IndexBufferObject);

		glBindVertexArray(m_VertexArrayObject);

		std::vector<float> buffer = {};

		for (auto vertex : GetVertices())
		{
			vertex->TransformData(buffer);
		}

		glBindBuffer(GL_ARRAY_BUFFER, m_VertexBufferObject);
		glBufferData(GL_ARRAY_BUFFER, buffer.size() * sizeof(float), buffer.data(), GL_STATIC_DRAW);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_IndexBufferObject);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(int), indices.data(), GL_STATIC_DRAW);

		GLsizei stride = 12 * sizeof(float); // 3 for position + 4 for color + 2 for texture coordinates + 3 for normals, in bytes

		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(0));
		glEnableVertexAttribArray(0);

		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(3 * sizeof(float)));
		glEnableVertexAttribArray(1);

		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(7 * sizeof(float)));
		glEnableVertexAttribArray(2);

		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, reinterpret_cast<void*>(9 * sizeof(float)));
		glEnableVertexAttribArray(3);

		glBindVertexArray(0);

	}
	void Mesh::BindData()
	{
		glBindVertexArray(m_VertexArrayObject);
	}
}