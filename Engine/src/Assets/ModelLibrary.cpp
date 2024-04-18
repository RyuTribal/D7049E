#include "pch.h"
#include "ModelLibrary.h"
#include "TextureLibrary.h"
#include "Renderer/Renderer.h"

#include <stb_image.h>

namespace Engine {
	ModelLibrary* ModelLibrary::s_Instance = nullptr;

	Ref<AssetSource> ModelLibrary::LoadModel(const std::string& file_path)
	{
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

		const aiScene* scene = m_Importer.ReadFile(absolute_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace );


		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		
			HVE_CORE_ERROR_TAG("Assimp", "Error: {0}", m_Importer.GetErrorString());
		}

		Ref<AssetSource> asset = CreateRef<AssetSource>();
		asset->scene = scene;

		m_Library[absolute_path] = asset;

		return asset;
	}

	Ref<Mesh> ModelLibrary::CreateMesh(const std::string& file_path, UUID* entity)
	{
		Ref<AssetSource> mesh_asset = LoadModel(file_path);

		if (mesh_asset == nullptr)
		{
			return nullptr;
		}

		std::vector<MeshNode> nodes;
		std::vector<Submesh> meshes;
		std::vector<Ref<Material>> materials;

		std::string absolute_path = std::filesystem::absolute(file_path).string();
		const std::string directory = absolute_path.substr(0, absolute_path.find_last_of('/'));

		int vertex_count = 0, index_count = 0;
		uint32_t root_node_index = ProcessNode(mesh_asset->scene->mRootNode, mesh_asset->scene, mesh_asset->transformed, nodes, meshes, vertex_count, index_count, entity, nullptr);

		// Materials
		Ref<Texture2D> WhiteTexture = Renderer::GetWhiteTexture();
		Ref<Texture2D> BlueTexture = Renderer::GetBlueTexture();
		if (mesh_asset->scene->HasMaterials())
		{
			auto scene = mesh_asset->scene;
			materials.resize(mesh_asset->scene->mNumMaterials);
			for (uint32_t i = 0; i < mesh_asset->scene->mNumMaterials; i++)
			{
				auto aiMaterial = scene->mMaterials[i];
				auto aiMaterialName = aiMaterial->GetName();
				Ref<Material> material = CreateRef<Material>(Renderer::GetShaderLibrary()->Get("default_static_pbr"));
				materials[i] = material;

				HVE_CORE_TRACE_TAG("Model Library","  {0} (Index = {1})", aiMaterialName.data, i);

				aiString aiTexPath;
				uint32_t textureCount = aiMaterial->GetTextureCount(aiTextureType_DIFFUSE);
				HVE_CORE_TRACE_TAG("Model Library", "    TextureCount = {0}", textureCount);
				
				glm::vec3 albedoColor(0.8f);
				float emission = 0.0f;
				aiColor3D aiColor, aiEmission;
				if (aiMaterial->Get(AI_MATKEY_COLOR_DIFFUSE, aiColor) == AI_SUCCESS)
					albedoColor = { aiColor.r, aiColor.g, aiColor.b };

				if (aiMaterial->Get(AI_MATKEY_COLOR_EMISSIVE, aiEmission) == AI_SUCCESS)
					emission = aiEmission.r;

				material->Set("u_MaterialUniforms.AlbedoColor", albedoColor);
				material->Set("u_MaterialUniforms.Emission", emission);

				float roughness, metalness;
				if (aiMaterial->Get(AI_MATKEY_ROUGHNESS_FACTOR, roughness) != aiReturn_SUCCESS)
					roughness = 0.5f;

				if (aiMaterial->Get(AI_MATKEY_REFLECTIVITY, metalness) != aiReturn_SUCCESS)
					metalness = 0.0f;

				HVE_CORE_TRACE_TAG("Model Library","    COLOR = {0}, {1}, {2}", aiColor.r, aiColor.g, aiColor.b);
				HVE_CORE_TRACE_TAG("Model Library","    ROUGHNESS = {0}", roughness);
				HVE_CORE_TRACE_TAG("Model Library","    METALNESS = {0}", metalness);
				bool hasAlbedoMap = aiMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &aiTexPath) == AI_SUCCESS;
				bool fallback = !hasAlbedoMap;

				if (hasAlbedoMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						spec.Format = ImageFormat::RGBA8;
						spec.Width = aiTexEmbedded->mWidth;
						spec.Height = aiTexEmbedded->mHeight;
						Ref<Texture2D> texture = Texture2D::Create(spec);
						texture->SetData(aiTexEmbedded->pcData, 1);
						texture_id = TextureLibrary::Get()->LoadTexture(texture);
					}
					else
					{
						std::string texturePath = directory + '/' + std::string(aiTexPath.data);
						HVE_CORE_TRACE_TAG("Model Library", "    Albedo map path = {0}", texturePath);
						texture_id = TextureLibrary::Get()->LoadTexture(texturePath);
					}

					Ref<Texture2D> texture = TextureLibrary::Get()->GetTexture(texture_id);
					if (texture && texture->IsLoaded())
					{
						material->Set("u_AlbedoTexture", texture, TextureSlots::Albedo);
						material->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(1.0f));
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}

				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No albedo map");
					material->Set("u_AlbedoTexture", WhiteTexture, TextureSlots::Albedo);
				}


				// Normal maps
				bool hasNormalMap = aiMaterial->GetTexture(aiTextureType_NORMALS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasNormalMap;
				if (hasNormalMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						spec.Format = ImageFormat::RGBA8;
						spec.Width = aiTexEmbedded->mWidth;
						spec.Height = aiTexEmbedded->mHeight;
						Ref<Texture2D> texture = Texture2D::Create(spec);
						texture->SetData(aiTexEmbedded->pcData, 1);
						texture_id = TextureLibrary::Get()->LoadTexture(texture);
					}
					else
					{
						std::string texturePath = directory + '/' + std::string(aiTexPath.data);
						HVE_CORE_TRACE_TAG("Model Library", "    Normal map path = {0}", texturePath);
						texture_id = TextureLibrary::Get()->LoadTexture(texturePath);
					}

					Ref<Texture2D> texture = TextureLibrary::Get()->GetTexture(texture_id);
					if (texture && texture->IsLoaded())
					{
						material->Set("u_NormalTexture", texture, TextureSlots::Normal);
						material->Set("u_MaterialUniforms.UseNormalMap", true);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No normal map");
					material->Set("u_NormalTexture", BlueTexture, TextureSlots::Normal);
					material->Set("u_MaterialUniforms.UseNormalMap", false);
				}


				// Roughness map
				bool hasRoughnessMap = aiMaterial->GetTexture(aiTextureType_SHININESS, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasRoughnessMap;
				if (hasRoughnessMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						spec.Format = ImageFormat::RGBA8;
						spec.Width = aiTexEmbedded->mWidth;
						spec.Height = aiTexEmbedded->mHeight;
						Ref<Texture2D> texture = Texture2D::Create(spec);
						texture->SetData(aiTexEmbedded->pcData, 1);
						texture_id = TextureLibrary::Get()->LoadTexture(texture);
					}
					else
					{
						std::string texturePath = directory + '/' + std::string(aiTexPath.data);
						HVE_CORE_TRACE_TAG("Model Library", "    Roughness map path = {0}", texturePath);
						texture_id = TextureLibrary::Get()->LoadTexture(texturePath);
					}

					Ref<Texture2D> texture = TextureLibrary::Get()->GetTexture(texture_id);
					if (texture && texture->IsLoaded())
					{
						material->Set("u_RoughnessTexture", texture, TextureSlots::Roughness);
						material->Set("u_MaterialUniforms.Roughness", 1.0f);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No roughness map");
					material->Set("u_RoughnessTexture", WhiteTexture, TextureSlots::Roughness);
					material->Set("u_MaterialUniforms.Roughness", roughness);
				}

				//Metalness

				bool metalnessTextureFound = false;
				for (uint32_t p = 0; p < aiMaterial->mNumProperties; p++)
				{
					auto prop = aiMaterial->mProperties[p];

					if (prop->mType == aiPTI_String)
					{
						uint32_t strLength = *(uint32_t*)prop->mData;
						std::string str(prop->mData + 4, strLength);

						std::string key = prop->mKey.data;
						if (key == "$raw.ReflectionFactor|file")
						{
							UUID texture_id;
							TextureSpecification spec;
							if (auto aiTexEmbedded = scene->GetEmbeddedTexture(str.data()))
							{
								spec.Format = ImageFormat::RGBA8;
								spec.Width = aiTexEmbedded->mWidth;
								spec.Height = aiTexEmbedded->mHeight;
								Ref<Texture2D> texture = Texture2D::Create(spec);
								texture->SetData(aiTexEmbedded->pcData, 1);
								texture_id = TextureLibrary::Get()->LoadTexture(texture);
							}
							else
							{
								std::string texturePath = directory + '/' + std::string(aiTexPath.data);
								HVE_CORE_TRACE_TAG("Model Library", "    Metalness map path = {0}", texturePath);
								texture_id = TextureLibrary::Get()->LoadTexture(texturePath);
							}

							Ref<Texture2D> texture = TextureLibrary::Get()->GetTexture(texture_id);
							if (texture && texture->IsLoaded())
							{
								material->Set("u_MetalnessTexture", texture, TextureSlots::Metalness);
								material->Set("u_MaterialUniforms.Metalness", 1.0f);
							}
							else
							{
								HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
							}
							break;
						}
					}
				}
				fallback = !metalnessTextureFound;
				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No metalness map");
					material->Set("u_MetalnessTexture", WhiteTexture, TextureSlots::Metalness);
					material->Set("u_MaterialUniforms.Metalness", metalness);

				}
			}
			HVE_CORE_TRACE_TAG("Model Library", "------------------------");
		}
		else
		{
			Ref<Material> material = CreateRef<Material>(Renderer::GetShaderLibrary()->Get("default_static_pbr"));
			material->Set("u_MaterialUniforms.AlbedoColor", glm::vec3(0.8f));
			material->Set("u_MaterialUniforms.Emission", 0.0f);
			material->Set("u_MaterialUniforms.Metalness", 0.0f);
			material->Set("u_MaterialUniforms.Roughness", 0.8f);
			material->Set("u_MaterialUniforms.UseNormalMap", false);
			material->Set("u_AlbedoTexture", WhiteTexture, TextureSlots::Albedo);
			material->Set("u_MetalnessTexture", WhiteTexture, TextureSlots::Metalness);
			material->Set("u_RoughnessTexture", WhiteTexture, TextureSlots::Roughness);
			materials.push_back(material);
		}

		Ref<Mesh> new_mesh = CreateRef<Mesh>(nodes, meshes, materials, root_node_index, vertex_count, index_count);

		MeshMetaData meta_data{};
		meta_data.MeshPath = absolute_path;
		//meta_data.ShaderPath = std::filesystem::absolute(shader_path).string(); // This part should be fixed with the creation of asset packs and such

		new_mesh->SetMetaData(meta_data);

		mesh_asset->transformed = true;

		return new_mesh;
	}

	uint32_t ModelLibrary::ProcessNode(aiNode* node, const aiScene* scene, bool transformed, std::vector<MeshNode>& node_destination, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity, aiMatrix4x4* parent_matrix)
	{
		MeshNode mesh_node{};
		mesh_node.Name = node->mName.C_Str();
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			mesh_node.Submeshes.push_back(ProcessMesh(mesh, scene, mesh_destination, vertex_count, index_count, entity));
			mesh_destination[mesh_node.Submeshes[mesh_node.Submeshes.size() - 1]].LocalTransform = ConvertMatrix(node->mTransformation);
			if (parent_matrix != nullptr && !transformed)
			{
				mesh_destination[mesh_node.Submeshes[mesh_node.Submeshes.size() - 1]].WorldTransform = ConvertMatrix(*parent_matrix) * ConvertMatrix(node->mTransformation);
			}
			else
			{
				mesh_destination[mesh_node.Submeshes[mesh_node.Submeshes.size() - 1]].WorldTransform = ConvertMatrix(node->mTransformation);
			}
		}

		if (parent_matrix != nullptr)
		{
			node->mTransformation = *parent_matrix * node->mTransformation;
		}
	
		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			mesh_node.Children.push_back(ProcessNode(node->mChildren[i], scene, transformed, node_destination, mesh_destination, vertex_count, index_count, entity, &node->mTransformation));
		}

		node_destination.push_back(mesh_node);
		return node_destination.size() - 1;
	}

	uint32_t ModelLibrary::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Submesh>& mesh_destination, int& vertex_count, int& index_count, UUID* entity)
	{
		mesh_destination.push_back(Submesh());
		mesh_destination[mesh_destination.size() - 1].MeshName = mesh->mName.C_Str();
		mesh_destination[mesh_destination.size() - 1].MaterialIndex = mesh->mMaterialIndex;
		HVE_CORE_TRACE_TAG("Mesh", "Meterial Index: {}", mesh->mMaterialIndex);
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

			if (mesh->mTangents)
			{
				vertex.tangent.x = mesh->mTangents[i].x;
				vertex.tangent.y = mesh->mTangents[i].y;
				vertex.tangent.z = mesh->mTangents[i].z;
			}

			if (mesh->mBitangents)
			{
				vertex.bitangent.x = mesh->mBitangents[i].x;
				vertex.bitangent.y = mesh->mBitangents[i].y;
				vertex.bitangent.z = mesh->mBitangents[i].z;
			}

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
			{ ShaderDataType::Float3, "a_tangent" },
			{ ShaderDataType::Float3, "a_bitangent" },
			{ ShaderDataType::Int, "a_entity_id"}
			});
		vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		mesh_destination[mesh_destination.size() - 1].VertexArray->AddVertexBuffer(vertexBuffer);
		mesh_destination[mesh_destination.size() - 1].VertexArray->SetIndexBuffer(indexBuffer);

		vertex_count += (int)vertices.size();
		index_count += (int)indices.size();

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


}
