#pragma once

namespace Engine {
	class HVec3
	{
	public:

		HVec3(float arr[3]);

		HVec3(float x, float y, float z);

		float GetX()
		{
			return m_arr[0];
		}
		float GetY()
		{
			return m_arr[1];
		}
		float GetZ()
		{
			return m_arr[2];
		}

	private:
		float m_arr[3];
	};

}

