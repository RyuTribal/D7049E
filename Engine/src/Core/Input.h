#pragma once

#include "Core/Base.h"


namespace Engine
{
	class Input
	{
	public:
		inline static bool IsKeyPressed(int keycode) { return s_Instance->IsKeyPressedImpl(keycode); }
		inline static bool IsMouseButtonPressed(int button) { return s_Instance->IsMouseButtonPressedImpl(button); }
		inline static void ClearKeyStates() { s_Instance->ClearKeyStatesImpl(); }
		inline static std::pair<float, float> GetMousePosition() { return s_Instance->GetMousePositionImpl(); }
		inline static float GetMouseX() { return s_Instance->GetMouseXImpl(); }
		inline static float GetMouseY() { return s_Instance->GetMouseYImpl(); }
		inline static void SetLockMouseMode(bool lock_mouse) { s_Instance->SetLockMouseModeImpl(lock_mouse); }

	protected:
		virtual bool IsKeyPressedImpl(int keycode) = 0;
		virtual void ClearKeyStatesImpl() = 0;
		virtual void SetLockMouseModeImpl(bool lock_mouse) = 0;
		virtual bool IsMouseButtonPressedImpl(int button) = 0;
		virtual std::pair<float, float> GetMousePositionImpl() = 0;
		virtual float GetMouseXImpl() = 0;
		virtual float GetMouseYImpl() = 0;
	private:
		static Input* s_Instance;
	};
}
