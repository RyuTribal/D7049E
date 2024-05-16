#pragma once
#include "HPhysicsScene.h"


namespace Engine {

	//namespace BroadPhaseLayers {	// TODO: structure this a bit better. Own file perhaps
	//	static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
	//	static constexpr JPH::BroadPhaseLayer MOVING(1);
	//	static constexpr JPH::uint NUM_LAYERS(2);
	//};

	//namespace Layers {
	//	static constexpr JPH::ObjectLayer NON_MOVING = 0;
	//	static constexpr JPH::ObjectLayer MOVING = 1;
	//	static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	//};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
	


	class PhysicsEngine
	{
	public:
		static PhysicsEngine* Get()
		{
			if (s_Instance)
			{
				return s_Instance;
			}
			s_Instance = new PhysicsEngine();
			return s_Instance;
		}
		void Init(int allocationSize);
		void Shutdown();
		//HBodyID createBox(JPH::Vec3 dimensions, JPH::RVec3 position, JPH::EMotionType movability, bool activate);
		

		void Step(float deltaTime);
		void OnRuntimeStart(int collisionSteps, int integrationSubStep);
		void OnRuntimeStop();

		SceneID CreateScene(Scene* scene, float allocationSize);
		HPhysicsScene* GetScene(SceneID sceneID);
		HPhysicsScene* GetCurrentScene();
		void SwitchScene(SceneID sceneID);

	private:
		static PhysicsEngine* s_Instance;

		//JPH::TempAllocatorImpl* m_temp_allocator;
		//JPH::JobSystemThreadPool* m_job_system;
		/*static Ref<JPH::PhysicsSystem> s_physics_system;
		static Ref<MyContactListener> s_contact_listener;


		static BPLayerInterfaceImpl s_broad_phase_layer_interface;
		static ObjectVsBroadPhaseLayerFilterImpl s_object_vs_broadphase_layer_filter;
		static ObjectLayerPairFilterImpl s_object_vs_object_layer_filter;

		static JPH::BodyInterface* s_body_interface;*/


		inline static std::map<SceneID, HPhysicsScene*> s_sceneMap = std::map<SceneID, HPhysicsScene*>();
		static HPhysicsScene* s_current_scene;

	};

}

