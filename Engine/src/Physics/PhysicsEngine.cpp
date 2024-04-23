#include "pch.h"
#include "PhysicsEngine.h"




namespace Engine {

	


	struct JoltData
	{
		JPH::TempAllocator* TemporariesAllocator;
		std::unique_ptr<JPH::JobSystemThreadPool> JobThreadPool;

		std::string LastErrorMessage = "";
	};

	static JoltData* s_JoltData = nullptr;

	static void JoltTraceCallback(const char* format, ...)
	{
		va_list list;
		va_start(list, format);
		char buffer[1024];
		vsnprintf(buffer, sizeof(buffer), format, list);

		s_JoltData->LastErrorMessage = buffer;
		HVE_CORE_TRACE_TAG("Physics", buffer);
	}

#ifdef JPH_ENABLE_ASSERTS

	static bool JoltAssertFailedCallback(const char* expression, const char* message, const char* file, uint32_t line)
	{
		HVE_CORE_FATAL_TAG("Physics", "{}:{}: ({}) {}", file, line, expression, message != nullptr ? message : "");
		return true;
	}
#endif

	JPH::RVec3 PhysicsEngine::makeRVec3(HVec3 arr)
	{
		return JPH::RVec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
	JPH::Vec3 PhysicsEngine::makeVec3(HVec3 arr)
	{
		return JPH::Vec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
	HVec3 PhysicsEngine::makeHVec3(JPH::Vec3 arr)
	{
		return HVec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
	//HVec3 PhysicsEngine::makeHVec3(JPH::RVec3 arr)
	//{
	//	return HVec3(arr.GetX(), arr.GetY(), arr.GetZ());
	//}
	JPH::EMotionType PhysicsEngine::makeEMotionType(HEMotionType movability)
	{
		switch (movability)
		{
			case HEMotionType::Static:
				return JPH::EMotionType::Static;
			case HEMotionType::Kinematic:
				return JPH::EMotionType::Kinematic;
			case HEMotionType::Dynamic:
				return JPH::EMotionType::Dynamic;
			default:
				return JPH::EMotionType::Static;
		}
	}
	HEMotionType PhysicsEngine::makeHEMotionType(JPH::EMotionType movability)
	{
		switch (movability)
		{
			case JPH::EMotionType::Static:
				return HEMotionType::Static;
			case JPH::EMotionType::Kinematic:
				return HEMotionType::Kinematic;
			case JPH::EMotionType::Dynamic:
				return HEMotionType::Dynamic;
			default:
				return HEMotionType::Static;
		}
	}

	JPH::BodyID PhysicsEngine::makeBodyID(HBodyID id)
	{
		return JPH::BodyID(id.GetIndexAndSequenceNumber());
	}

	HBodyID PhysicsEngine::makeHBodyID(JPH::BodyID id)
	{
		return HBodyID(id.GetIndexAndSequenceNumber());
	}

	PhysicsEngine* PhysicsEngine::s_Instance = nullptr;

	void PhysicsEngine::Init(int allocationSize)
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JoltTraceCallback;

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_JoltData = new JoltData();

		//s_JoltData->TemporariesAllocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		//s_JoltData->JobThreadPool = std::unique_ptr<JPH::JobSystemThreadPool> ( 
		//	new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1)
		//);

		this->m_temp_allocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		this->m_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

		m_physics_system = CreateRef<JPH::PhysicsSystem>();
		(this->m_physics_system)->Init(
			this->cMaxBodies,
			this->cNumBodyMutexes,
			this->cMaxBodyPairs,
			this->cMaxContactConstraints,
			this->m_broad_phase_layer_interface,
			this->m_object_vs_broadphase_layer_filter,
			this->m_object_vs_object_layer_filter
		);


		m_physics_system->SetBodyActivationListener(&(this->m_body_activation_listener));

		m_physics_system->SetContactListener(&(this->m_contact_listener));

		this->m_body_interface = &(m_physics_system->GetBodyInterface());

	}

	void PhysicsEngine::Shutdown()
	{
		delete s_JoltData->TemporariesAllocator;
		delete s_JoltData;
		delete JPH::Factory::sInstance;

		JPH::UnregisterTypes();

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	HBodyID PhysicsEngine::createBox(HVec3 dimensions, HVec3 position, HEMotionType movability, bool activate)
	{
		// conversion
		JPH::Vec3 dim = PhysicsEngine::makeVec3(dimensions);
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);

		JPH::BoxShapeSettings box_shape_settings(dim);
		JPH::ShapeSettings::ShapeResult box_shape_result = box_shape_settings.Create();
		if (box_shape_result.HasError())
		{
			s_JoltData->LastErrorMessage = box_shape_result.GetError();
		}
		assert(!box_shape_result.HasError());	// TODO: remove?
		JPH::ShapeRefC box_shape = box_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()


		JPH::BodyCreationSettings box_settings;
		if (movability == HEMotionType::Static)
		{
			box_settings = JPH::BodyCreationSettings(box_shape, pos, JPH::Quat::sIdentity(), mov, Layers::NON_MOVING);
		}
		else
		{
			box_settings = JPH::BodyCreationSettings(box_shape, pos, JPH::Quat::sIdentity(), mov, Layers::MOVING);

			// Note: it is possible to have more than two layers but that has not been implemented yet
		}

		JPH::BodyID box_id;
		// Add it to the world
		if (activate)
		{
			box_id = (this->m_body_interface)->CreateAndAddBody(box_settings, JPH::EActivation::Activate);
		}
		else
		{
			box_id = (this->m_body_interface)->CreateAndAddBody(box_settings, JPH::EActivation::DontActivate);
		}

		return PhysicsEngine::makeHBodyID(box_id);
	}

	HBodyID PhysicsEngine::createSphere(float radius, HVec3 position, HEMotionType movability, bool activate)
	{
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);

		JPH::SphereShapeSettings sphere_shape_settings(radius);		// TODO: I think we can add material here
		JPH::ShapeSettings::ShapeResult sphere_shape_result = sphere_shape_settings.Create();
		if (sphere_shape_result.HasError())
		{
			s_JoltData->LastErrorMessage = sphere_shape_result.GetError();
		}
		JPH::ShapeRefC sphere_shape = sphere_shape_result.Get();
		JPH::BodyCreationSettings sphere_settings;
		if (mov == JPH::EMotionType::Static)
		{
			sphere_settings = JPH::BodyCreationSettings(sphere_shape, pos, JPH::Quat::sIdentity(), mov, Layers::NON_MOVING);
		}
		else
		{
			sphere_settings = JPH::BodyCreationSettings(sphere_shape, pos, JPH::Quat::sIdentity(), mov, Layers::MOVING);
		}
		JPH::BodyID sphere_id;
		if (activate)
		{
			sphere_id = (this->m_body_interface)->CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
		}
		else
		{
			sphere_id = (this->m_body_interface)->CreateAndAddBody(sphere_settings, JPH::EActivation::DontActivate);
		}

		return PhysicsEngine::makeHBodyID(sphere_id);
	}

	void PhysicsEngine::insertObjectByID(HBodyID id, bool activate)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);
		if (activate)
		{
			(this->m_body_interface)->AddBody(jolt_id, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->AddBody(jolt_id, JPH::EActivation::DontActivate);
		}
	}

	void PhysicsEngine::setPosition(HBodyID id, HVec3 position, bool activate)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);

		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		if (activate)
		{
			(this->m_body_interface)->SetPosition(jolt_id, pos, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->SetPosition(jolt_id, pos, JPH::EActivation::DontActivate);
		}
	}

	void PhysicsEngine::setLinearVelocity(HBodyID id, HVec3 velocity)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);
		JPH::Vec3 vel = PhysicsEngine::makeVec3(velocity);

		(this->m_body_interface)->SetLinearVelocity(jolt_id, vel);
	}

	void PhysicsEngine::setAngularVelocity(HBodyID id, HVec3 velocity)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);
		JPH::Vec3 vel = PhysicsEngine::makeVec3(velocity);

		(this->m_body_interface)->SetAngularVelocity(jolt_id, vel);
	}

	void PhysicsEngine::setLinearAndAngularVelocity(HBodyID id, HVec3 linaerVelocity, HVec3 angularVelocity)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);
		JPH::Vec3 lVel = PhysicsEngine::makeVec3(linaerVelocity);
		JPH::Vec3 aVel = PhysicsEngine::makeVec3(angularVelocity);

		(this->m_body_interface)->SetLinearAndAngularVelocity(jolt_id, lVel, aVel);
	}

	void PhysicsEngine::optimizeBroadPhase()
	{
		(this->m_physics_system)->OptimizeBroadPhase();
	}

	void PhysicsEngine::step(int integrationSubSteps)
	{
		(this->m_physics_system)->Update(
			this->cDeltaTime,
			this->cCollisionSteps,
			integrationSubSteps,
			(this->m_temp_allocator),
			(this->m_job_system)
		);
	}

	void PhysicsEngine::step(int collisionSteps, int integrationSubSteps)
	{
		(this->m_physics_system)->Update(
			this->cDeltaTime,
			collisionSteps,
			integrationSubSteps,
			(this->m_temp_allocator),
			(this->m_job_system)
		);
	}

	void PhysicsEngine::removeBody(HBodyID id)
	{
		JPH::BodyID h_id = PhysicsEngine::makeBodyID(id);
		(this->m_body_interface)->RemoveBody(h_id);
	}

	void PhysicsEngine::destoryBody(HBodyID id)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);

		(this->m_body_interface)->DestroyBody(jolt_id);
	}

	bool PhysicsEngine::isActive(HBodyID id)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);

		return (this->m_body_interface)->IsActive(jolt_id);
	}

	HVec3 PhysicsEngine::getCenterOfMassPosition(HBodyID id)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);

		JPH::RVec3 vec = (this->m_body_interface)->GetCenterOfMassPosition(jolt_id);
		return PhysicsEngine::makeHVec3(vec);
	}

	HVec3 PhysicsEngine::getLinearVelocity(HBodyID id)
	{
		JPH::BodyID jolt_id = PhysicsEngine::makeBodyID(id);

		JPH::Vec3 vec = (this->m_body_interface)->GetLinearVelocity(jolt_id);
		return PhysicsEngine::makeHVec3(vec);
	}

	// TODO: delete tmp function below
	void PhysicsEngine::tmpRunner()
	{
		PhysicsEngine* engin = PhysicsEngine::Get();
		
		engin->Init(10);
		HBodyID box_id = engin->createBox(HVec3(100.0f, 1.0f, 100.0f), HVec3(0.0, -1.0, 0.0), HEMotionType::Static, false);
		//HBodyID sphere_id = engin->createSphere(0.5f, HVec3(0.0, 2.0, 0.0), HEMotionType::Dynamic, true);
		HBodyID sphere_id = engin->createBox(HVec3(1.0f, 1.0f, 1.0f), HVec3(0.0, 2.0, 0.0), HEMotionType::Dynamic, true);
		engin->setLinearVelocity(sphere_id, HVec3(0.0f, -5.0f, 0.0f));
		engin->optimizeBroadPhase();

		int stepCounter = 0;
		while (engin->isActive(sphere_id) && stepCounter < 200)
		{
			++stepCounter;
			HVec3 position = engin->getCenterOfMassPosition(sphere_id);
			HVec3 velocity = engin->getLinearVelocity(sphere_id);
			std::cout << "Step " << stepCounter << ": Position = (" << position.GetX() << ", " << position.GetY() << ", " << position.GetZ() << "), Velocity = (" << velocity.GetX() << ", " << velocity.GetY() << ", " << velocity.GetZ() << ")" << std::endl;
			engin->step(1);
			
		}
		std::cout << "Finished the tmp simulation" << std::endl;


	}
}
