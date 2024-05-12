#pragma once
#include <Engine.h>

namespace EditorPanels {

	class ProjectSettings
	{
	public:
		static void Init();
		static void Render(Engine::Ref<Engine::Camera> editor_camera);
		static void Shutdown();
		
	};
}
