#pragma once
#include <Renderer/Texture.h>


namespace Engine {
	class TextureLibrary
	{
	public:
		static TextureLibrary* Get()
		{
			if (!s_Instance)
			{
				s_Instance = new TextureLibrary();
			}

			return s_Instance;
		}

		TextureLibrary() = default;
		~TextureLibrary() = default;

		UUID LoadTexture(const std::string& path);
		Ref<Texture2D> GetTexture(UUID& id);
		UUID LoadTexture(Ref<Texture2D> texture);

	private:
		std::unordered_map<UUID, Ref<Texture2D>> m_Library;
		std::unordered_map<std::string, UUID> m_PathLibrary;
		static TextureLibrary* s_Instance;
	};
}
