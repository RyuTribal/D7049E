#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <Renderer/Mesh.h>


namespace Engine {

	struct AssetSource
	{
		const aiScene* scene;
		UUID id{};
	};

	class ModelLibrary
	{
	public:
		static ModelLibrary* Get()
		{
			if (!s_Instance)
			{
				s_Instance = new ModelLibrary();
			}

			return s_Instance;
		}

		ModelLibrary() = default;
		~ModelLibrary() = default;

		Ref<AssetSource> LoadModel(const std::string& file_path);
		Ref<Mesh> CreateMesh(const std::string& file_path, UUID* entity, const std::string& shader_path);

	private:
		uint32_t ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshNode>& node_destination, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity, aiMatrix4x4* parent_matrix, std::vector<Ref<Material>>& material_destination, const std::string& directory, const std::string& shader_path);
		uint32_t ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity, std::vector<Ref<Material>>& material_destination, const std::string& directory, const std::string& shader_path);
		void LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& directory, Ref<Material>& material);
		glm::mat4 ConvertMatrix(const aiMatrix4x4& aiMat);

	private:
		static ModelLibrary* s_Instance;
		std::unordered_map<std::string, Ref<AssetSource>> m_Library;
		Assimp::Importer m_Importer;
	};
}
