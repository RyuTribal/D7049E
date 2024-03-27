#pragma once
#include <Engine.h>

namespace TestApp {
	class Silver : public Engine::Phong {
	public:
		Silver() : Engine::Phong() {
			// Values taken from http://devernay.free.fr/cours/opengl/materials.html
			SetAmbient(glm::vec3(0.19225f, 0.19225f, 0.19225f));
			SetDiffuse(glm::vec3(0.50754f, 0.50754f, 0.50754f));
			SetSpecular(glm::vec3(0.508273f, 0.508273f, 0.508273));
			SetShininess(0.4f * 128);
		}
	};
}