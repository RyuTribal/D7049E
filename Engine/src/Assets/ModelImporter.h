#pragma once

#include <Renderer/Mesh.h>
#include "AssetManager.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace Engine {

	class ModelImporter
	{
	public:
		static Ref<MeshSource> ImportSource(AssetHandle handle, const AssetMetadata& metadata);
		static Ref<Mesh> Import(AssetHandle handle, const AssetMetadata& metadata);
		static void Serialize(AssetHandle handle, const AssetMetadata& metadata);

	private:
		static uint32_t ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshNode>& node_destination, std::vector<Submesh>& mesh_destination, Ref<MeshSource> mesh_source, Math::BoundingBox& global_bounds, int& vertex_count, int& index_count);
		static uint32_t ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Submesh>& mesh_destination, Ref<MeshSource> mesh_source, int& vertex_count, int& index_count);
		static glm::mat4 ConvertMatrix(const aiMatrix4x4& aiMat);
		
	};
}
