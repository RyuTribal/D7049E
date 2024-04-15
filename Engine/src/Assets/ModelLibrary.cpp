#include "pch.h"
#include "ModelLibrary.h"
#include "TextureLibrary.h"

namespace Engine {
	ModelLibrary* ModelLibrary::s_Instance = nullptr;

	Ref<AssetSource> ModelLibrary::LoadModel(const std::string& file_path)
	{
		auto iter = m_Library.find(file_path);
		if (iter != m_Library.end())
		{
			return m_Library[file_path];
		}

		if (!std::filesystem::exists(file_path))
		{
			return nullptr;
		}

		const std::string file_ending = std::filesystem::path(file_path).extension().string();

		if (file_ending != ".fbx" && file_ending != ".FBX" && file_ending != ".gltf" && file_ending != ".glb")
		{
			HVE_CORE_ERROR_TAG("Model Library", "Tried to load a model file of type {0}, which is not supported", file_ending);
			return nullptr;
		}

		const std::string absolute_path = std::filesystem::absolute(file_path).string();
		
		HVE_CORE_TRACE_TAG("Model Library", "Opening file: {0}", absolute_path);

		const aiScene* scene = m_Importer.ReadFile(absolute_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);


		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		
			HVE_CORE_ERROR_TAG("Assimp", "Error: {0}", m_Importer.GetErrorString());
		}

		Ref<AssetSource> asset = CreateRef<AssetSource>();
		asset->scene = scene;

		m_Library[absolute_path] = asset;

		return asset;
	}

	Ref<Mesh> ModelLibrary::CreateMesh(const std::string& file_path, UUID* entity, const std::string& shader_path)
	{
		Ref<AssetSource> mesh_asset = LoadModel(file_path);

		if (mesh_asset == nullptr)
		{
			return nullptr;
		}

		std::vector<MeshNode> nodes;
		std::vector<Submesh> meshes;
		std::vector<Ref<Material>> materials;

		const std::string absolute_path = std::filesystem::absolute(file_path).string();
		const std::string directory = absolute_path.substr(0, absolute_path.find_last_of('/'));

		int vertex_count = 0, index_count = 0;

		uint32_t root_node_index = ProcessNode(mesh_asset->scene->mRootNode, mesh_asset->scene, nodes, meshes, vertex_count, index_count, entity, nullptr, materials, directory, shader_path);

		return CreateRef<Mesh>(nodes, meshes, materials, root_node_index, vertex_count, index_count);
	}

	uint32_t ModelLibrary::ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshNode>& node_destination, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity, aiMatrix4x4* parent_matrix, std::vector<Ref<Material>>& material_destination, const std::string& directory, const std::string& shader_path)
	{
		MeshNode mesh_node{};
		mesh_node.Name = node->mName.C_Str();

		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			mesh_node.Submeshes.push_back(ProcessMesh(mesh, scene, mesh_destination, vertex_count, index_count, entity, material_destination, directory, shader_path));
			mesh_destination[mesh_node.Submeshes[mesh_node.Submeshes.size() - 1]].LocalTransform = ConvertMatrix(node->mTransformation);
			if (parent_matrix != nullptr)
			{
				mesh_destination[mesh_node.Submeshes[mesh_node.Submeshes.size() - 1]].WorldTransform = ConvertMatrix(*parent_matrix) * ConvertMatrix(node->mTransformation);
			}
		}

		if (parent_matrix != nullptr)
		{
			node->mTransformation = *parent_matrix * node->mTransformation;
		}
	
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			mesh_node.Children.push_back(ProcessNode(node->mChildren[i], scene, node_destination, mesh_destination, vertex_count, index_count, entity, &node->mTransformation, material_destination, directory, shader_path));
		}

		node_destination.push_back(mesh_node);
		return node_destination.size() - 1;
	}

	uint32_t ModelLibrary::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity, std::vector<Ref<Material>>& material_destination, const std::string& directory, const std::string& shader_path)
	{
		mesh_destination.push_back(Submesh());
		mesh_destination[mesh_destination.size() - 1].MeshName = mesh->mName.C_Str();
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};
			vertex.coordinates.x = mesh->mVertices[i].x;
			vertex.coordinates.y = mesh->mVertices[i].y;
			vertex.coordinates.z = mesh->mVertices[i].z;

			vertex.normal.x = mesh->mNormals[i].x;
			vertex.normal.y = mesh->mNormals[i].y;
			vertex.normal.z = mesh->mNormals[i].z;

			if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
			{
				vertex.texture_coordinates.x = mesh->mTextureCoords[0][i].x;
				vertex.texture_coordinates.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texture_coordinates = glm::vec2(0.0f, 0.0f);
			}

			vertex.entity_id = entity != nullptr ? (int)*entity : -1;

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
				indices.push_back(face.mIndices[j]);
		}

		mesh_destination[mesh_destination.size() - 1].VertexArray = VertexArray::Create();


		auto vertexBuffer = VertexBuffer::Create(vertices.size() * sizeof(Vertex));
		vertexBuffer->SetLayout({
			{ ShaderDataType::Float3, "a_coords" },
			{ ShaderDataType::Float4, "a_colors" },
			{ ShaderDataType::Float2, "a_texture_coords" },
			{ ShaderDataType::Float3, "a_normals" },
			{ ShaderDataType::Int, "a_entity_id"}
			});
		vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		mesh_destination[mesh_destination.size() - 1].VertexArray->AddVertexBuffer(vertexBuffer);
		mesh_destination[mesh_destination.size() - 1].VertexArray->SetIndexBuffer(indexBuffer);

		vertex_count += (int)vertices.size();
		index_count += (int)indices.size();


		// Material //////
		if (mesh->mMaterialIndex >= 0)
		{
			aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

			Ref<Material> new_material = CreateRef<Material>();

			new_material->SetProgram(CreateRef<ShaderProgram>(shader_path));

			LoadMaterialTextures(material, aiTextureType_DIFFUSE, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_NORMALS, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_SHININESS, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_METALNESS, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_SPECULAR, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_HEIGHT, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_OPACITY, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_AMBIENT_OCCLUSION, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_REFLECTION, directory, new_material);
			LoadMaterialTextures(material, aiTextureType_EMISSIVE, directory, new_material);

			material_destination.push_back(new_material);

			mesh_destination[mesh_destination.size() - 1].MaterialIndex = (uint32_t)material_destination.size() - 1;
		}


		return (uint32_t)(mesh_destination.size()) - 1;
	}


	glm::mat4 ModelLibrary::ConvertMatrix(const aiMatrix4x4& aiMat)
	{
		return {
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
		};
	}

	void ModelLibrary::LoadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& directory, Ref<Material>& material)
	{
		for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
		{
			aiString str;
			mat->GetTexture(type, i, &str);

			std::string texturePath = directory + '/' + std::string(str.C_Str());
			UUID textureId = TextureLibrary::Get()->LoadTexture(texturePath);

			switch (type)
			{
				case aiTextureType_DIFFUSE:
					material->SetAlbedoTexture(textureId);
					break;
				case aiTextureType_NORMALS:
					material->SetNormalTexture(textureId);
					break;
				case aiTextureType_SHININESS:
					material->SetRoughnessTexture(textureId);
					break;
				case aiTextureType_METALNESS:
					material->SetMetalnessTexture(textureId);
					break;
				case aiTextureType_SPECULAR:
					material->SetSpecularTexture(textureId);
					break;
				case aiTextureType_HEIGHT:
					material->SetHeightTexture(textureId);
					break;
				case aiTextureType_OPACITY:
					material->SetOpacityTexture(textureId);
					break;
				case aiTextureType_AMBIENT_OCCLUSION:
					material->SetAOTexture(textureId);
					break;
				case aiTextureType_REFLECTION:
					material->SetRefractionTexture(textureId);
					break;
				case aiTextureType_EMISSIVE:
					material->SetEmissiveTexture(textureId);
					break;
				default:
					// Dont know how to handle unknowns yet, we'll get there when we need to
					break;
			}
		}
	}


}
