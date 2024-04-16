#include "PhysicsEngine.h"


PhysicsEngine::PhysicsEngine(int allocationSize)
{
	RegisterDefaultAllocator();

	// Install callbacks
	Trace = TraceImpl;
	JPH_IF_ENABLE_ASSERTS(AssertFailed = AssertFailedImpl;)

	// Create a factory
	Factory::sInstance = new Factory();

	// Register all Jolt physics types
	RegisterTypes();

	this->m_temp_allocator = new TempAllocatorImpl(allocationSize * 1024 * 1024);

	this->m_job_system = new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, thread::hardware_concurrency() - 1);
		

	(this->m_physics_system).Init(
		this->cMaxBodies, 
		this->cNumBodyMutexes, 
		this->cMaxBodyPairs, 
		this->cMaxContactConstraints, 
		this->m_broad_phase_layer_interface, 
		this->m_object_vs_broadphase_layer_filter, 
		this->m_object_vs_object_layer_filter
	);

		
	m_physics_system.SetBodyActivationListener(&(this->m_body_activation_listener));

	m_physics_system.SetContactListener(&(this->m_contact_listener));

	this->m_body_interface = &(m_physics_system.GetBodyInterface());
}

PhysicsEngine::~PhysicsEngine()
{
	UnregisterTypes();

	// Destroy the factory
	delete Factory::sInstance;
	Factory::sInstance = nullptr;
}

BodyID PhysicsEngine::createBox(Vec3 dimensions, RVec3 position, EMotionType movability, bool activate)
{
	BoxShapeSettings box_shape_settings(dimensions);
	ShapeSettings::ShapeResult box_shape_result = box_shape_settings.Create();
	assert(!box_shape_result.HasError());
	ShapeRefC box_shape = box_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()

	BodyCreationSettings* box_settings;
	if (movability == EMotionType::Static)
	{
		box_settings = new BodyCreationSettings(box_shape, position, Quat::sIdentity(), movability, Layers::NON_MOVING);
	}
	else
	{
		box_settings = new BodyCreationSettings(box_shape, position, Quat::sIdentity(), movability, Layers::MOVING);
		// Note: it is possible to have more than two layers but that has not been implemented yet
	}

	Body* box = (this->m_body_interface)->CreateBody(*box_settings); // Note that if we run out of bodies this can return nullptr

	// Add it to the world
	if (activate)
	{
		(this->m_body_interface)->AddBody(box->GetID(), EActivation::Activate);
	}
	else
	{
		(this->m_body_interface)->AddBody(box->GetID(), EActivation::DontActivate);
	}
	return box->GetID();
}

BodyID PhysicsEngine::createSphere(float radius, RVec3 position, EMotionType movability, bool activate)
{
	BodyCreationSettings* sphere_settings;
	if (movability == EMotionType::Static)
	{
		sphere_settings = new BodyCreationSettings(new SphereShape(0.5f), position, Quat::sIdentity(), movability, Layers::NON_MOVING);

	}
	else
	{
		sphere_settings = new BodyCreationSettings(new SphereShape(0.5f), position, Quat::sIdentity(), movability, Layers::NON_MOVING);
	}
	BodyID sphere_id;
	if (activate)
	{
		sphere_id = (this->m_body_interface)->CreateAndAddBody(*sphere_settings, EActivation::Activate);
	}
	else
	{
		sphere_id = (this->m_body_interface)->CreateAndAddBody(*sphere_settings, EActivation::Activate);
	}
	return sphere_id;
}

void PhysicsEngine::insertObjectByID(BodyID id, bool activate)
{
	if (activate)
	{
		(this->m_body_interface)->AddBody(id, EActivation::Activate);
	}
	else
	{
		(this->m_body_interface)->AddBody(id, EActivation::DontActivate);
	}
}

void PhysicsEngine::setPosition(BodyID id, RVec3 position, bool activate)
{
	if (activate)
	{
		(this->m_body_interface)->SetPosition(id, position, EActivation::Activate);
	}
	else
	{
		(this->m_body_interface)->SetPosition(id, position, EActivation::DontActivate);
	}
}

void PhysicsEngine::setLinearVelocity(BodyID id, Vec3 velocity)
{
	(this->m_body_interface)->SetLinearVelocity(id, velocity);
}

void PhysicsEngine::setAngularVelocity(BodyID id, Vec3 velocity)
{
	(this->m_body_interface)->SetAngularVelocity(id, velocity);
}

void PhysicsEngine::setLinearAndAngularVelocity(BodyID id, Vec3 linaerVelocity, Vec3 angularVelocity)
{
	(this->m_body_interface)->SetLinearAndAngularVelocity(id, linaerVelocity, angularVelocity);
}

void PhysicsEngine::optimizeBroadPhase()
{
	(this->m_physics_system).OptimizeBroadPhase();
}

void PhysicsEngine::step(int integrationSubSteps)
{
	(this->m_physics_system).Update(
		this->cDeltaTime, 
		this->cCollisionSteps, 
		integrationSubSteps, 
		(this->m_temp_allocator),
		(this->m_job_system)
	);
}

void PhysicsEngine::step(int collisionSteps, int integrationSubSteps)
{
	(this->m_physics_system).Update(
		this->cDeltaTime,
		collisionSteps,
		integrationSubSteps,
		(this->m_temp_allocator),
		(this->m_job_system)
	);
}

void PhysicsEngine::removeBody(BodyID id)
{
	(this->m_body_interface)->RemoveBody(id);
}

void PhysicsEngine::destoryBody(BodyID id)
{
	(this->m_body_interface)->DestroyBody(id);
}

bool PhysicsEngine::isActive(BodyID id)
{
	return (this->m_body_interface)->IsActive(id);
}

RVec3 PhysicsEngine::getCenterOfMassPosition(BodyID id)
{
	return (this->m_body_interface)->GetCenterOfMassPosition(id);
}

Vec3 PhysicsEngine::getLinearVelocity(BodyID id)
{
	return (this->m_body_interface)->GetLinearVelocity(id);
}

// TODO: delete tmp function below
static void PhysicsEngine::tmpRunner()
{
	PhysicsEngine engin(10);
	engin.createBox(Vec3(100.0f, 1.0f, 100.0f), RVec3(0.0_r, -1.0_r, 0.0_r), EMotionType::Static, true);
	BodyID sphere_id = engin.createSphere(0.5f, RVec3(0.0_r, 2.0_r, 0.0_r), EMotionType::Dynamic true);
	engin.setLinearVelocity(sphere_id, Vec3(0.0f, -5.0f, 0.0f));
	engin.optimizeBroadPhase();

	int stepCounter = 0;
	while (engin.isActive(sphere_id))
	{
		++stepCounter;
		engin.step(1);
		RVec3 position = engin.getCenterOfMassPosition(sphere_id);
		Vec3 velocity = engin.getLinearVelocity(sphere_id);
		cout << "Step " << stepCounter << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << endl;

	}
	cout << "Finished the tmp simulation" << endl;
	
}
