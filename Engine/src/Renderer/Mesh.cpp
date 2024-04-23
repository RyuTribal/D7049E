#include "pch.h"
#include "Mesh.h"

namespace Engine {
	Mesh::Mesh(Ref<MeshSource> source) : m_MeshSource(source)
	{
	
	}

	Mesh::~Mesh()
	{
	}
	void Mesh::SetTransform(glm::mat4 transform)
	{
		m_Transform = transform;
	}
}
