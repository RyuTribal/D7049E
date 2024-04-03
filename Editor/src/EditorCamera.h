#pragma once

#include <Engine.h>

namespace Editor {
	class EditorCamera {
	public:
		EditorCamera() {}
		~EditorCamera() = default;
	private:
		Engine::UUID m_ID;
		Engine::Ref<Engine::Camera> m_Camera = Engine::CreateRef<Engine::Camera>(Engine::CameraType::PERSPECTIVE);
	};
}