#pragma once
#include <unordered_map>
#include <Engine.h>


namespace Editor {
	class EditorResources
	{
	public:
		static void Init();

		static std::unordered_map<std::string, Engine::Ref<Engine::Texture2D>> FileIcons;
	};
}
