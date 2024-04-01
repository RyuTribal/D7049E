#include "pch.h"
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t> indices)
	{
		m_VertexArray = VertexArray::Create();

		std::vector<float> buffer = {};
		for (auto &vertex : vertices)
		{
			vertex.TransformData(buffer);
		}
		auto vertexBuffer = VertexBuffer::Create(buffer.size() * sizeof(float));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_coords" },
			{ ShaderDataType::Float4, "a_colors" },
			{ ShaderDataType::Float2, "a_texture_coords" },
			{ ShaderDataType::Float3, "a_normals" }
			});
		vertexBuffer->SetData(buffer.data(), buffer.size() * sizeof(float));

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);
	}
	Mesh::~Mesh()
	{
	}

	void Mesh::Bind() const {
		m_VertexArray->Bind();
	}

	void Mesh::Unbind() const {
		m_VertexArray->Unbind();
	}
}