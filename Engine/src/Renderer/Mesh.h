#pragma once
#include <glad/gl.h>
#include "VertexArray.h"
#include "Material.h"


namespace Engine {

	struct Vertex
	{
		glm::vec3 coordinates = { 0.f, 0.f, 0.f };
		glm::vec4 color = { 1.f, 1.f, 1.f, 1.f };
		glm::vec2 texture_coordinates = { 1.f, 1.f };
		glm::vec3 normal = { 0.0f, 0.0f, 0.0f };
		glm::vec3 tangent = { 0.0f, 0.0f, 0.0f };
		glm::vec3 bitangent = { 0.0f, 0.0f, 0.0f };
		uint32_t entity_id = -1;
	};

	class Submesh
	{
	public:
		uint32_t MaterialIndex = -1;
		Ref<VertexArray> VertexArray;

		glm::mat4 LocalTransform{ 1.0f };

		glm::mat4 WorldTransform{ 1.0f };

		std::string MeshName;
	};

	struct MeshNode
	{
		uint32_t Parent = 0xffffffff;
		std::vector<uint32_t> Children;
		std::vector<uint32_t> Submeshes;
		std::string Name;
		inline bool IsRoot() const { return Parent == 0xffffffff; }
	};

	struct MeshMetaData
	{
		std::string MeshPath = "";
		std::string ShaderPath = "";
	};

	class Mesh
	{
	public:
		Mesh() = default;
		Mesh(std::vector<MeshNode> nodes, std::vector<Submesh> meshes, std::vector<Ref<Material>> materials, uint32_t root_node, int vertex_count, int index_count);
		~Mesh();

		int VertexSize() { return m_VertexCount; }
		int IndexSize() { return m_IndexCount; }

		glm::mat4 GetTransform() { return m_Transform; }
		void SetTransform(glm::mat4 transform);

		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		std::vector<Ref<Material>>& GetMaterials() { return m_Materials; }

		void SetShader(const std::string& path);

		MeshMetaData& GetMetaData() { return m_MetaData; }
		void SetMetaData(MeshMetaData meta_data) { m_MetaData = meta_data; }

	private:
		MeshMetaData m_MetaData{};
		std::vector<MeshNode> m_Nodes;
		std::vector<Submesh> m_Submeshes;
		uint32_t m_RootNode;
		int m_VertexCount = 0;
		int m_IndexCount = 0;
		std::vector<Ref<Material>> m_Materials;
		glm::mat4 m_Transform;
	};
}
