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

	class MeshSource : public Asset
	{
	public:
		int VertexSize() { return m_VertexCount; }
		int IndexSize() { return m_IndexCount; }
		std::vector<Submesh>& GetSubmeshes() { return m_Submeshes; }
		std::vector<Ref<Material>>& GetMaterials() { return m_Materials; }
		void SetNodes(std::vector<MeshNode> nodes) { m_Nodes = nodes; }
		void SetSubmeshes(std::vector<Submesh> submeshes) { m_Submeshes = submeshes; }
		void SetMaterials(std::vector<Ref<Material>> materials) { m_Materials = materials; }

		static AssetType GetStaticType() { return AssetType::MeshSource; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }
	private:
		std::vector<MeshNode> m_Nodes;
		std::vector<Submesh> m_Submeshes;
		std::vector<Ref<Material>> m_Materials;
		uint32_t m_RootNode;
		int m_VertexCount = 0;
		int m_IndexCount = 0;
	};

	class Mesh: public Asset
	{
	public:
		Mesh(Ref<MeshSource> source);
		~Mesh();

		glm::mat4 GetTransform() { return m_Transform; }
		void SetTransform(glm::mat4 transform);

		Ref<MeshSource> GetMeshSource() { return m_MeshSource; }

		static AssetType GetStaticType() { return AssetType::Mesh; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

	private:
		Ref<MeshSource> m_MeshSource;
		glm::mat4 m_Transform;
	};
}
