#pragma once
#include <Engine.h>

namespace Editor {
	class Cuboid : public Engine::Mesh {
	public:
		Cuboid(float height, float width, float depth);

	private:
		float h, w, d;
	};
}