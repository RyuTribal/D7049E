#pragma once


namespace Engine {
	class PhysicsEngine
	{
	public:
		static PhysicsEngine* Get()
		{
			if (s_Instance)
			{
				return s_Instance;
			}
			return new PhysicsEngine();
		}
		void Init();
		void Shutdown();
		void Update();

	private:
		static PhysicsEngine* s_Instance;
	};

}

