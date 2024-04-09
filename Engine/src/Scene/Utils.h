#pragma once

#include "Core/ModelLibrary.h"
#include "Scene.h"

namespace Engine {
	//static void CreateEntitiesFromObjectFiles(const std::string& file_path, Scene* context)
	//{
	//	const aiScene* model_scene = ModelLibrary::Get()->LoadModel(file_path);

	//	ProcessNode(model_scene->mRootNode, model_scene, context, nullptr);
	//}


	//static void ProcessNode(aiNode* node, const aiScene* scene, Scene* context, EntityHandle* parent)
	//{
	//	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	//	{
	//		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
	//		CreateEntity(mesh, context, parent);
	//	}
	//	
	//	/*for (unsigned int i = 0; i < node->mNumChildren; i++)
	//	{
	//		processNode(node->mChildren[i], scene);
	//	}*/
	//}

	//static EntityHandle* CreateEntity(aiMesh* mesh, Scene* context, EntityHandle* parent)
	//{

	//}
}
