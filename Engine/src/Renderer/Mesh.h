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

	struct Triangle
	{
		Vertex V0, V1, V2;
	};

	class Submesh
	{
	public:
		uint32_t Index = -1;
		uint32_t MaterialIndex = -1;
		Ref<VertexArray> VertexArray;

		glm::mat4 LocalTransform{ 1.0f };

		glm::mat4 WorldTransform{ 1.0f };

		std::string MeshName;

		Math::BoundingBox Bounds;
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
		const std::vector<Triangle> GetTriangleCache(uint32_t index) const { return m_TriangleCache.at(index); }
		void AddTriangleCache(uint32_t index, Triangle triangle) { m_TriangleCache[index].push_back(triangle); }
		Math::BoundingBox* GetBounds() { return &m_Bounds; }


		static AssetType GetStaticType() { return AssetType::MeshSource; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }
	private:
		std::vector<MeshNode> m_Nodes;
		std::vector<Submesh> m_Submeshes;
		std::vector<Ref<Material>> m_Materials;
		std::unordered_map<uint32_t, std::vector<Triangle>> m_TriangleCache;
		uint32_t m_RootNode;
		int m_VertexCount = 0;
		int m_IndexCount = 0;
		Math::BoundingBox m_Bounds;
	};

	class Mesh: public Asset
	{
	public:
		Mesh(Ref<MeshSource> source);
		~Mesh();

		glm::mat4 GetTransform() { return m_Transform; }
		void SetTransform(glm::mat4 transform);

		Ref<MeshSource> GetMeshSource() { return m_MeshSource; }
		void SetMeshSource(Ref<MeshSource> mesh_source) { m_MeshSource = mesh_source; }

		static AssetType GetStaticType() { return AssetType::Mesh; } // Good for templated functions
		AssetType GetType() const { return GetStaticType(); }

	private:
		Ref<MeshSource> m_MeshSource;
		glm::mat4 m_Transform;
	};
}
