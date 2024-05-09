#include "pch.h"
#include "ModelImporter.h"
#include "Renderer/Renderer.h"
#include "Project/Project.h"
#include "TextureImporter.h"
#include "Serialization/YAMLSerializer.h"
#include <stb_image.h>

static const uint32_t s_MeshImportFlags =
	aiProcess_CalcTangentSpace |        // Create binormals/tangents just in case
	aiProcess_Triangulate |             // Make sure we're triangles
	aiProcess_GenNormals |              // Make sure we have legit normals
	aiProcess_GenUVCoords |             // Convert UVs if required 
	//		aiProcess_OptimizeGraph |
	aiProcess_OptimizeMeshes |          // Batch draws where possible
	aiProcess_JoinIdenticalVertices |
	aiProcess_LimitBoneWeights |        // If more than N (=4) bone weights, discard least influencing bones and renormalise sum to 1
	aiProcess_ValidateDataStructure |   // Validation
	aiProcess_PreTransformVertices | // this pre applies transformations which might mess up aniamtion data so possibly remove later
	aiProcess_GlobalScale;

namespace Engine {

	Ref<MeshSource> ModelImporter::ImportSource(AssetHandle handle, const AssetMetadata& metadata)
	{
		Ref<MeshSource> meshSource = CreateRef<MeshSource>();
		Math::BoundingBox globalBounds;
		std::filesystem::path full_asset_path = Project::GetFullFilePath(metadata.FilePath);
		HVE_CORE_INFO_TAG("Mesh loader", "Loading mesh: {0}", full_asset_path);

		Assimp::Importer importer;
		importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);
		const aiScene* scene = importer.ReadFile(full_asset_path.string(), s_MeshImportFlags);

		if (!scene)
		{
			HVE_CORE_ERROR_TAG("Mesh", "Failed to load mesh file: {0}", full_asset_path.string());
			return nullptr;
		}

		std::vector<MeshNode> nodes;
		std::vector<Submesh> meshes;
		std::vector<Ref<Material>> materials;
		const std::filesystem::path directory = full_asset_path.parent_path();

		int vertex_count = 0, index_count = 0;
		uint32_t root_node_index = ProcessNode(scene->mRootNode, scene, nodes, meshes, meshSource, globalBounds, vertex_count, index_count);

		// Materials
		Ref<Texture2D> WhiteTexture = Renderer::GetWhiteTexture();
		Ref<Texture2D> BlueTexture = Renderer::GetBlueTexture();
		Ref<Texture2D> BlackTexture = Renderer::GetBlackTexture();
		Ref<Texture2D> GrayTexture = Renderer::GetGrayTexture();
		if (scene->HasMaterials())
		{
			materials.resize(scene->mNumMaterials);
			for (uint32_t i = 0; i < scene->mNumMaterials; i++)
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

				if (aiMaterial->Get(AI_MATKEY_METALLIC_FACTOR, metalness) != aiReturn_SUCCESS)
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
						
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory /  aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Albedo map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Albedo map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);
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

				// Specular Color maps

				bool hasSpecularMap = aiMaterial->GetTexture(aiTextureType_SPECULAR, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasSpecularMap;

				if (hasSpecularMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{

						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Specular color map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Specular color map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);
					if (texture && texture->IsLoaded())
					{
						material->Set("u_SpecularTexture", texture, TextureSlots::SpecularColor);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}

				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No specular color map");
					material->Set("u_SpecularTexture", WhiteTexture, TextureSlots::SpecularColor);
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
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Normal map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Normal map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);

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
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Roughness map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Roughness map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);
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
				bool hasMetalMap = aiMaterial->GetTexture(aiTextureType_UNKNOWN, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasMetalMap;
				if (hasMetalMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Metalness map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Metalness map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);

					if (texture && texture->IsLoaded())
					{
						material->Set("u_MetalnessTexture", texture, TextureSlots::Metalness);
						material->Set("u_MaterialUniforms.Metalness", metalness);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No metalness map");
					material->Set("u_MetalnessTexture", WhiteTexture, TextureSlots::Metalness);
					material->Set("u_MaterialUniforms.Metalness", 0.0f);
				}


				// AOTexture
				bool hasAOMap = aiMaterial->GetTexture(aiTextureType_LIGHTMAP, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasAOMap;
				if (hasAOMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    AO map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    AO map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);

					if (texture && texture->IsLoaded())
					{
						material->Set("u_AOTexture", texture, TextureSlots::AO);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No ao map");
					material->Set("u_AOTexture", WhiteTexture, TextureSlots::AO);
				}


				// Emission
				bool hasEmissionMap = aiMaterial->GetTexture(aiTextureType_EMISSIVE, 0, &aiTexPath) == AI_SUCCESS;
				fallback = !hasEmissionMap;
				if (hasEmissionMap)
				{
					UUID texture_id;
					TextureSpecification spec;
					if (auto aiTexEmbedded = scene->GetEmbeddedTexture(aiTexPath.C_Str()))
					{
						Buffer image_data;
						int width, height, channels;
						if (aiTexEmbedded->mHeight == 0)
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth, &width, &height, &channels, 0);
						}
						else
						{
							image_data.Data = stbi_load_from_memory(reinterpret_cast<unsigned char*>(aiTexEmbedded->pcData), aiTexEmbedded->mWidth * aiTexEmbedded->mHeight, &width, &height, &channels, 0);
						}

						spec.Format = channels == 3 ? ImageFormat::RGB8 : ImageFormat::RGBA8;
						spec.Width = (uint32_t)width;
						spec.Height = (uint32_t)height;
						image_data.Size = spec.Width * spec.Height * channels;

						texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(spec, image_data);
					}
					else
					{
						auto texturePath = directory / aiTexPath.C_Str();
						if (!std::filesystem::exists(texturePath))
						{
							HVE_CORE_TRACE_TAG("Model Library", "    Emission map path = {0} --> NOT FOUND", texturePath);
							texturePath = directory / texturePath.filename();
						}
						HVE_CORE_TRACE_TAG("Model Library", "    Emission map path = {0}", texturePath);
						auto new_texture = TextureImporter::Import2DWithPath(texturePath.string());
						if (new_texture)
						{
							texture_id = Project::GetActive()->GetDesignAssetManager()->CreateMemoryOnlyAsset<Texture2D>(new_texture);
						}
					}

					Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(texture_id);

					if (texture && texture->IsLoaded())
					{
						material->Set("u_EmissionTexture", texture, TextureSlots::Emissive);
					}
					else
					{
						HVE_CORE_ERROR_TAG("Mesh", "Could not load texture: {0}", aiTexPath.C_Str());
						fallback = true;
					}
				}

				if (fallback)
				{
					HVE_CORE_TRACE_TAG("Model Library", "    No emission map");
					material->Set("u_EmissionTexture", BlackTexture, TextureSlots::Emissive);
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
			material->Set("u_AOTexture", WhiteTexture, TextureSlots::AO);
			material->Set("u_EmissionTexture", BlackTexture, TextureSlots::Emissive);
			material->Set("u_SpecularTexture", WhiteTexture, TextureSlots::SpecularColor);
			materials.push_back(material);
		}

		meshSource->SetNodes(nodes);
		meshSource->SetSubmeshes(meshes);
		meshSource->SetMaterials(materials);
		meshSource->GetBounds()->ExpandBy(globalBounds);

		return meshSource;
	}

	uint32_t ModelImporter::ProcessNode(aiNode* node, const aiScene* scene, std::vector<MeshNode>& node_destination, std::vector<Submesh>& mesh_destination, Ref<MeshSource> mesh_source, Math::BoundingBox& global_bounds, int& vertex_count, int& index_count)
	{
		MeshNode mesh_node{};
		mesh_node.Name = node->mName.C_Str();
		for (unsigned int i = 0; i < node->mNumMeshes; i++)
		{
			aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
			uint32_t submesh_index = ModelImporter::ProcessMesh(mesh, scene, mesh_destination, mesh_source, vertex_count, index_count);
			mesh_node.Submeshes.push_back(submesh_index);
			global_bounds.ExpandBy(mesh_destination[submesh_index].Bounds);
		}

		for (unsigned int i = 0; i < node->mNumChildren; i++)
		{
			mesh_node.Children.push_back(ModelImporter::ProcessNode(node->mChildren[i], scene, node_destination, mesh_destination, mesh_source, global_bounds, vertex_count, index_count));
		}

		node_destination.push_back(mesh_node);
		return node_destination.size() - 1;
	}

	uint32_t ModelImporter::ProcessMesh(aiMesh* mesh, const aiScene* scene, std::vector<Submesh>& mesh_destination, Ref<MeshSource> mesh_source, int& vertex_count, int& index_count)
	{
		mesh_destination.push_back(Submesh());
		mesh_destination[mesh_destination.size() - 1].Index = mesh_destination.size() - 1;
		mesh_destination[mesh_destination.size() - 1].MeshName = mesh->mName.C_Str();
		mesh_destination[mesh_destination.size() - 1].MaterialIndex = mesh->mMaterialIndex;
		HVE_CORE_TRACE_TAG("Mesh", "Material Index: {}", mesh->mMaterialIndex);
		std::vector<Vertex> vertices;
		std::vector<uint32_t> indices;
		for (unsigned int i = 0; i < mesh->mNumVertices; i++)
		{
			Vertex vertex{};
			vertex.coordinates.x = mesh->mVertices[i].x;
			vertex.coordinates.y = mesh->mVertices[i].y;
			vertex.coordinates.z = mesh->mVertices[i].z;
			mesh_destination[mesh_destination.size() - 1].Bounds.ExpandBy(vertex.coordinates);

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
			else
			{
				vertex.bitangent = glm::cross(vertex.tangent, vertex.normal);
			}

			if (mesh->mTextureCoords[0])
			{
				vertex.texture_coordinates.x = mesh->mTextureCoords[0][i].x;
				vertex.texture_coordinates.y = mesh->mTextureCoords[0][i].y;
			}
			else
			{
				vertex.texture_coordinates = glm::vec2(0.0f, 0.0f);
			}

			vertices.push_back(vertex);
		}

		for (unsigned int i = 0; i < mesh->mNumFaces; i++)
		{
			aiFace face = mesh->mFaces[i];
			for (unsigned int j = 0; j < face.mNumIndices; j++)
			{
				indices.push_back(face.mIndices[j]);
			}
		}

		for (size_t i = 0; i < indices.size(); i += 3)
		{
			if (i + 2 < indices.size())
			{
				Triangle new_triangle{};
				new_triangle.V0 = vertices[indices[i]];
				new_triangle.V1 = vertices[indices[i + 1]];
				new_triangle.V2 = vertices[indices[i + 2]];

				mesh_source->AddTriangleCache(mesh_destination.size() - 1, new_triangle);
			}
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
			});
		vertexBuffer->SetData(vertices.data(), vertices.size() * sizeof(Vertex));

		auto indexBuffer = IndexBuffer::Create(indices.data(), indices.size());
		mesh_destination[mesh_destination.size() - 1].VertexArray->AddVertexBuffer(vertexBuffer);
		mesh_destination[mesh_destination.size() - 1].VertexArray->SetIndexBuffer(indexBuffer);

		vertex_count += (int)vertices.size();
		index_count += (int)indices.size();

		return (uint32_t)(mesh_destination.size()) - 1;
	}


	glm::mat4 ModelImporter::ConvertMatrix(const aiMatrix4x4& aiMat)
	{
		return {
		aiMat.a1, aiMat.b1, aiMat.c1, aiMat.d1,
		aiMat.a2, aiMat.b2, aiMat.c2, aiMat.d2,
		aiMat.a3, aiMat.b3, aiMat.c3, aiMat.d3,
		aiMat.a4, aiMat.b4, aiMat.c4, aiMat.d4
		};
	}


}
