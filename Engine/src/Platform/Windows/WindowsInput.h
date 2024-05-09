#pragma once
#include "Core/Input.h"

namespace Engine
{
	class WindowsInput : public Input
	{
	protected:
		bool IsKeyPressedImpl(int keycode) override;
		void SetLockMouseModeImpl(bool lock_mouse) override;
		bool IsMouseButtonPressedImpl(int button) override;
		float GetMouseXImpl() override;
		float GetMouseYImpl() override;
		std::pair<float, float> GetMousePositionImpl() override;
		void ClearKeyStatesImpl() override;

	private:
		bool b_IsLocked = false;
	};
}
