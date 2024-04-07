#include "pch.h"
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(std::vector<Vertex>& vertices, std::vector<uint32_t> indices)
	{
		m_VertexArray = VertexArray::Create();

		std::vector<VertexData> buffer = {};
		for (auto &vertex : vertices)
		{
			VertexData data{};
			data.coordinates = vertex.coordinates;
			data.color = vertex.color;
			data.normal = vertex.normal;
			data.texture_coordinates = vertex.texture_coordinates;
			data.entity_id = vertex.entity_id;
			buffer.push_back(data);
		}
		auto vertexBuffer = VertexBuffer::Create(buffer.size() * sizeof(VertexData));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_coords" },
			{ ShaderDataType::Float4, "a_colors" },
			{ ShaderDataType::Float2, "a_texture_coords" },
			{ ShaderDataType::Float3, "a_normals" },
			{ ShaderDataType::Int, "a_entity_id"}
			});
		vertexBuffer->SetData(buffer.data(), buffer.size() * sizeof(VertexData));

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		m_VertexArray->AddVertexBuffer(vertexBuffer);
		m_VertexArray->SetIndexBuffer(indexBuffer);

		m_VertexCount = (int)vertices.size();
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