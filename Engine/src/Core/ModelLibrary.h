#pragma once

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>


namespace Engine {
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

		const aiScene* LoadModel(const std::string& file_path);

	private:
		static ModelLibrary* s_Instance;
		std::unordered_map<std::string, Ref<aiScene>> m_Library;
	};
}
