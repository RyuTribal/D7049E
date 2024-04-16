#pragma once

#include "Core/PhysicsResources.cpp"



class PhysicsEngine
{
public:
	PhysicsEngine(int allocationSize);
	~PhysicsEngine();
	BodyID createBox(Vec3 dimensions, RVec3 position, EMotionType movability, bool activate);
	BodyID createSphere(float radius, RVec3 position, EMotionType movability, bool activate);
	void insertObjectByID(BodyID id, bool activate);
	void setPosition(BodyID id, RVec3 position, bool activate);
	void setLinearVelocity(BodyID id, Vec3 velocity);
	void setAngularVelocity(BodyID id, Vec3 velocity);
	void setLinearAndAngularVelocity(BodyID id, Vec3 linaerVelocity, Vec3 angularVelocity);
	void optimizeBroadPhase();
	void step(int cIntegrationSubSteps);
	void step(int collisionSteps, int integrationSubSteps);
	void removeBody(BodyID id);
	void destoryBody(BodyID id);
	bool isActive(BodyID id);
	RVec3 getCenterOfMassPosition(BodyID id);
	Vec3 getLinearVelocity(BodyID id);
	static void tmpRunner();

private:
	TempAllocatorImpl* m_temp_allocator; // (allocationSize * 1024 * 1024);

	JobSystemThreadPool* m_job_system; // (cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);

	PhysicsSystem m_physics_system;

	const uint cMaxBodies = 65536;

	const uint cNumBodyMutexes = 0;

	const uint cMaxBodyPairs = 65536;

	const uint cMaxContactConstraints = 10240;

	const float cDeltaTime = 1.0f / 60.0f;	// 60 Hz

	const int cCollisionSteps = 1;

	BPLayerInterfaceImpl m_broad_phase_layer_interface;
	ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
	ObjectLayerPairFilterImpl m_object_vs_object_layer_filter;

	MyBodyActivationListener m_body_activation_listener;

	MyContactListener m_contact_listener;

	BodyInterface* m_body_interface;
};
