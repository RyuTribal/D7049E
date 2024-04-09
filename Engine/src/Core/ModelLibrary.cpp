#include "pch.h"
#include "ModelLibrary.h"

namespace Engine {
	ModelLibrary* s_Instance = nullptr;

	const aiScene* ModelLibrary::LoadModel(const std::string& file_path)
	{
		auto iter = m_Library.find(file_path);
		if (iter == m_Library.end())
		{
			return m_Library[file_path].get();
		}

		if (!std::filesystem::exists(file_path))
		{
			return nullptr;
		}

		const std::string file_ending = std::filesystem::path(file_path).extension().string();

		if (file_ending != ".fbx" || file_ending != ".gltf" || file_ending != ".glb")
		{
			HVE_CORE_ERROR_TAG("Model Library", "Tried to load a model file of type {0}, which is not supported", file_ending);
		}

		const std::string absolute_path = std::filesystem::absolute(file_path).string();
		
		Assimp::Importer importer;
		const aiScene* scene = importer.ReadFile(absolute_path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);


		if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
		
			HVE_CORE_ERROR_TAG("Assimp", "Error: {0}", importer.GetErrorString());
		}

		return scene;
	}
}
