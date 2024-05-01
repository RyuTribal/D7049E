#pragma once


namespace Engine {

	class Scene;

	class PhysicsEngine
	{
	public:
		static void Init();
		static void Shutdown();

		static Ref<PhysicsScene> CreatePhysicsScene(const Ref<Scene> scene);
	};
}
