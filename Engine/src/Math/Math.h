#pragma once

#include <glm/glm.hpp>

namespace Engine::Math {

	bool DecomposeTransform(const glm::mat4& transform, glm::vec3& translation, glm::vec3& rotation, glm::vec3& scale);
	float RandomFloat(float begin, float end);
	int RandomInt(int begin, int end);

}

