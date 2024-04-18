#pragma once
#include "Auxiliary/HVec3.h"
#include "Auxiliary/HVec3.h"
#include "Auxiliary/HEMotionType.h"
#include "Auxiliary/HBodyID.h"


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
		void Init(int allocationSize);
		void Shutdown();
		void Update();
		//HBodyID createBox(JPH::Vec3 dimensions, JPH::RVec3 position, JPH::EMotionType movability, bool activate);		// TODO: change Vec3 to normal vector
		HBodyID createBox(HVec3 dimensions, HVec3 position, HEMotionType movability, bool activate);		// TODO: change Vec3 to normal vector
		HBodyID createSphere(float radius, HVec3 position, HEMotionType movability, bool activate);
		void insertObjectByID(HBodyID id, bool activate);
		void setPosition(HBodyID id, HVec3 position, bool activate);
		void setLinearVelocity(HBodyID id, HVec3 velocity);
		void setAngularVelocity(HBodyID id, HVec3 velocity);
		void setLinearAndAngularVelocity(HBodyID id, HVec3 linaerVelocity, HVec3 angularVelocity);
		void optimizeBroadPhase();
		void step(int integrationSubSteps);
		void step(int collisionSteps, int integrationSubSteps);
		void removeBody(HBodyID id);
		void destoryBody(HBodyID id);
		bool isActive(HBodyID id);
		HVec3 getCenterOfMassPosition(HBodyID id);
		HVec3 getLinearVelocity(HBodyID id);
		static void tmpRunner();

	private:
		static PhysicsEngine* s_Instance;

		JPH::TempAllocatorImpl* m_temp_allocator;
		JPH::JobSystemThreadPool* m_job_system;
		JPH::PhysicsSystem m_physics_system;

		const JPH::uint cMaxBodies = 65536;

		const JPH::uint cNumBodyMutexes = 0;

		const JPH::uint cMaxBodyPairs = 65536;

		const JPH::uint cMaxContactConstraints = 10240;

		const float cDeltaTime = 1.0f / 60.0f;	// 60 Hz

		const int cCollisionSteps = 1;

		BPLayerInterfaceImpl m_broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl m_object_vs_object_layer_filter;

		MyBodyActivationListener m_body_activation_listener;

		MyContactListener m_contact_listener;

		JPH::BodyInterface* m_body_interface;

		static JPH::RVec3 makeRVec3(HVec3 arr);
		static JPH::Vec3 makeVec3(HVec3 arr);
		static HVec3 makeHVec3(JPH::Vec3 arr);
		static HVec3 makeHVec3(JPH::RVec3 arr);
		static JPH::EMotionType makeEMotionType(HEMotionType movability);
		static HEMotionType makeHEMotionType(JPH::EMotionType movability);
		static JPH::BodyID makeBodyID(HBodyID id);
		static HBodyID makeHBodyID(JPH::BodyID id);


	};

}

