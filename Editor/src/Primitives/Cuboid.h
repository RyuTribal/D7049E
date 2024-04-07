#pragma once
#include <Engine.h>

namespace Editor {
	class Cuboid {
	public:
		Cuboid(float height, float width, float depth, Engine::UUID entity_id);

		Engine::Ref <Engine::Mesh> GetMesh() { return mesh; }

	private:
		float h, w, d;
		Engine::Ref<Engine::Mesh> mesh;
	};
}