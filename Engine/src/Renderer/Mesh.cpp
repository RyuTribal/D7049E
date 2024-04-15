#include "pch.h"
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(std::vector<MeshNode> nodes, std::vector<Submesh> meshes, std::vector<Ref<Material>> materials, uint32_t root_node, int vertex_count, int index_count) : m_Nodes(nodes), m_Submeshes(meshes), m_Materials(materials), m_RootNode(root_node), m_VertexCount(vertex_count), m_IndexCount(index_count)
	{
	
	}

	Mesh::~Mesh()
	{
	}
	void Mesh::SetTransform(glm::mat4 transform)
	{
		m_Transform = transform;
	}
	void Mesh::SetShader(const std::string& path)
	{
		for (size_t i = 0; i < m_Materials.size(); i++)
		{
			m_Materials[i]->SetProgram(CreateRef<ShaderProgram>(path));
		}
	}
}
