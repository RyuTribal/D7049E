#include "pch.h"
#include "TextureLibrary.h"

namespace Engine {

	TextureLibrary* TextureLibrary::s_Instance = nullptr;

	UUID TextureLibrary::LoadTexture(const std::string& path)
	{
		auto iter = m_PathLibrary.find(path);
		if (iter != m_PathLibrary.end())
		{
			return m_PathLibrary[path];
		}

		UUID new_id{};

		m_PathLibrary[path] = new_id;
		m_Library[new_id] = Texture2D::Create(path);
		return new_id;
	}
	Ref<Texture2D> TextureLibrary::GetTexture(UUID& id)
	{
		auto iter = m_Library.find(id);
		if (iter != m_Library.end())
		{
			return m_Library[id];
		}
		return nullptr;
	}
	UUID TextureLibrary::LoadTexture(Ref<Texture2D> texture)
	{
		UUID new_id{};
		std::string new_path = std::to_string(new_id);
		auto iter = m_PathLibrary.find(new_path);
		if (iter != m_PathLibrary.end())
		{
			return m_PathLibrary[new_path];
		}
		m_PathLibrary[new_path] = new_id;
		m_Library[new_id] = texture;
		return new_id;
	}
}
