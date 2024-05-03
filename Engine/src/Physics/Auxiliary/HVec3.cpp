#include "HVec3.h"

namespace Engine {

	HVec3::HVec3(float arr[3])
	{
		this->m_arr[0] = arr[0];
		this->m_arr[1] = arr[1];
		this->m_arr[2] = arr[2];
	}

	HVec3::HVec3(float x, float y, float z)
	{
		this->m_arr[0] = x;
		this->m_arr[1] = y;
		this->m_arr[2] = z;
	}
}
