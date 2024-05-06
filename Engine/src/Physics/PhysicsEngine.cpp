#include "pch.h"
#include "PhysicsEngine.h"
#include "PhysicsEngine.h"

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>


namespace Engine {

	struct JoltData
	{
		JPH::TempAllocator* TemporariesAllocator;
		std::unique_ptr<JPH::JobSystemThreadPool> JobThreadPool;

		std::string LastErrorMessage = "";

		JPH::uint cMaxBodies = 65536;

		JPH::uint cNumBodyMutexes = 0;

		JPH::uint cMaxBodyPairs = 65536;

		JPH::uint cMaxContactConstraints = 10240;

		//float cDeltaTime = 1.0f / 60.0f;	// 60 Hz

		int collisionSteps = 1;

		int integrationSubSteps = 1;

		bool hasOptimized = false;
		int numberOfBodies = 0;
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

	JPH::RVec3 PhysicsEngine::makeRVec3(glm::vec3 arr)
	{
		return JPH::RVec3(arr.x, arr.y, arr.z);
	}
	JPH::Vec3 PhysicsEngine::makeVec3(glm::vec3 arr)
	{
		return JPH::Vec3(arr.x, arr.y, arr.z);
	}
	glm::vec3 PhysicsEngine::makeGLMVec3(JPH::Vec3 arr)
	{
		return glm::vec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
	glm::vec4 PhysicsEngine::makeGLMVec4(JPH::Vec4 arr)
	{
		return glm::vec4(arr.GetX(), arr.GetY(), arr.GetZ(), arr.GetW());
	}
	glm::mat4x4 PhysicsEngine::makeMat4x4(JPH::RMat44 mat)
	{
		return glm::mat4x4{
			PhysicsEngine::makeGLMVec4(mat.GetColumn4(0)),
			PhysicsEngine::makeGLMVec4(mat.GetColumn4(1)),
			PhysicsEngine::makeGLMVec4(mat.GetColumn4(2)),
			PhysicsEngine::makeGLMVec4(mat.GetColumn4(3))
		};
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

	PhysicsEngine* PhysicsEngine::s_Instance = nullptr;

	void PhysicsEngine::Init(int allocationSize)
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JoltTraceCallback;

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_JoltData = new JoltData();

		s_JoltData->TemporariesAllocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		s_JoltData->JobThreadPool = std::unique_ptr<JPH::JobSystemThreadPool> (
			new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1)
		);

		//this->m_temp_allocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		//this->m_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);

		m_physics_system = CreateRef<JPH::PhysicsSystem>();
		(this->m_physics_system)->Init(
			s_JoltData->cMaxBodies,
			s_JoltData->cNumBodyMutexes,
			s_JoltData->cMaxBodyPairs,
			s_JoltData->cMaxContactConstraints,
			this->m_broad_phase_layer_interface,
			this->m_object_vs_broadphase_layer_filter,
			this->m_object_vs_object_layer_filter
		);

		m_physics_system->SetContactListener(this->m_contact_listener);

		/*m_physics_system->SetBodyActivationListener(&(this->m_body_activation_listener));

		m_physics_system->SetContactListener(&(this->m_contact_listener));*/

		this->m_body_interface = &(m_physics_system->GetBodyInterface());

	}

	void PhysicsEngine::Shutdown()
	{
		delete s_JoltData->TemporariesAllocator;
		delete s_JoltData;

		JPH::UnregisterTypes();

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;
	}

	HBodyID PhysicsEngine::CreateBody(Entity* entity)	//TODO: return differently if we're adding multiple shapes to the entity
	{
		BoxColliderComponent* boxComponent = entity->GetComponent<BoxColliderComponent>();
		if (boxComponent)
		{
			glm::vec3 posistion(0, 0, 0);		// TODO: ask about position
			return PhysicsEngine::Get()->CreateBox( 
				entity->GetID(), 
				boxComponent->HalfSize, 
				posistion,
				boxComponent->MotionType, 
				boxComponent->Offset, 
				true 
			);
		}
		SphereColliderComponent* sphereComponent = entity->GetComponent<SphereColliderComponent>();
		if (sphereComponent)
		{
			glm::vec3 posistion(0, 0, 0);
			return PhysicsEngine::Get()->CreateSphere(
				entity->GetID(),
				sphereComponent->Radius,
				posistion,
				sphereComponent->MotionType,
				sphereComponent->Offset,
				true
			);
		}
		CharacterControllerComponent* characterComponent = entity->GetComponent <CharacterControllerComponent>(); {}
		if (characterComponent)
		{
			return PhysicsEngine::Get()->CreateCharacter(
				entity->GetID(),
				characterComponent->Mass,
				characterComponent->HalfHeight,
				characterComponent->Radius,
				characterComponent->Position,
				characterComponent->UserData
			);
		}
	}

	HBodyID PhysicsEngine::CreateBox(UUID entity_id, glm::vec3 dimensions, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate)
	{
		// conversion
		JPH::Vec3 dim = PhysicsEngine::makeVec3(dimensions);
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);
		JPH::Vec3 jolt_offset = PhysicsEngine::makeVec3(offset);

		JPH::BoxShapeSettings* box_shape_settings = new JPH::BoxShapeSettings(dim);		// TODO: I think we can add material here
		JPH::RotatedTranslatedShapeSettings offsetShapeSettings(jolt_offset, JPH::Quat::sIdentity(), box_shape_settings);

		JPH::ShapeSettings::ShapeResult box_shape_result = offsetShapeSettings.Create();
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

		box_shape_settings->Release();

		s_JoltData->numberOfBodies++;
		s_JoltData->hasOptimized = false;
		return HBodyID(entity_id, box_id);
	}

	HBodyID PhysicsEngine::CreateSphere(UUID entity_id, float radius, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate)
	{
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);
		JPH::Vec3 jolt_offset = PhysicsEngine::makeVec3(offset);

		JPH::SphereShapeSettings* sphere_shape_settings = new JPH::SphereShapeSettings(radius);		// TODO: I think we can add material here
		JPH::RotatedTranslatedShapeSettings offsetShapeSettings(jolt_offset, JPH::Quat::sIdentity(), sphere_shape_settings);
		JPH::ShapeSettings::ShapeResult sphere_shape_result = offsetShapeSettings.Create();
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

		sphere_shape_settings->Release();

		s_JoltData->numberOfBodies++;
		s_JoltData->hasOptimized = false;
		return HBodyID(entity_id ,sphere_id);
	}

	HBodyID PhysicsEngine::CreateCharacter(UUID entity_id, float mass, float halfHeight, float radius, glm::vec3 position, std::uint64_t userData)
	{
		Scope<JPH::CharacterSettings> character_settings =  CreateScope<JPH::CharacterSettings>();
		character_settings->mMass = mass;
		//character_settings->mMaxStrength = strength;
		character_settings->mShape = JPH::CapsuleShapeSettings(halfHeight, radius)		// TODO: add material
			.Create().Get();

		JPH::Character character = JPH::Character(
			character_settings.get(),
			PhysicsEngine::makeRVec3(position),
			JPH::Quat::sIdentity(),
			userData,
			this->m_physics_system.get()
		);
		character.SetLayer(Layers::MOVING);
		character.AddToPhysicsSystem();
		
		return HBodyID(entity_id, character.GetBodyID());
	}

	void PhysicsEngine::InsertObjectByID(UUID entity_id, bool activate)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		if (activate)
		{
			(this->m_body_interface)->AddBody(jolt_id, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->AddBody(jolt_id, JPH::EActivation::DontActivate);
		}
	}

	void PhysicsEngine::SetPosition(UUID entity_id, glm::vec3 position, bool activate)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

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

	void PhysicsEngine::SetLinearVelocity(UUID entity_id, glm::vec3& velocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 vel = PhysicsEngine::makeVec3(velocity);

		(this->m_body_interface)->SetLinearVelocity(jolt_id, vel);
	}

	void PhysicsEngine::SetAngularVelocity(UUID entity_id, glm::vec3& velocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 vel = PhysicsEngine::makeVec3(velocity);

		(this->m_body_interface)->SetAngularVelocity(jolt_id, vel);
	}

	void PhysicsEngine::SetLinearAndAngularVelocity(UUID entity_id, glm::vec3& linaerVelocity, glm::vec3& angularVelocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 lVel = PhysicsEngine::makeVec3(linaerVelocity);
		JPH::Vec3 aVel = PhysicsEngine::makeVec3(angularVelocity);

		(this->m_body_interface)->SetLinearAndAngularVelocity(jolt_id, lVel, aVel);
	}

	void PhysicsEngine::AddLinearVelocity(UUID entity_id, glm::vec3& velocity)
	{

	}

	void PhysicsEngine::AddLinearImpulse(UUID entity_id, glm::vec3& impulse)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 imp = PhysicsEngine::makeVec3(impulse);

		(this->m_body_interface)->AddImpulse(jolt_id, imp);
	}

	void PhysicsEngine::AddAngularImpulse(UUID entity_id, glm::vec3& impulse)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 imp = PhysicsEngine::makeVec3(impulse);

		(this->m_body_interface)->AddAngularImpulse(jolt_id, imp);
	}

	void PhysicsEngine::AddLinearAndAngularImpulse(UUID entity_id, glm::vec3& linear, glm::vec3& angular)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 linear_imp = PhysicsEngine::makeVec3(linear);
		JPH::Vec3 angular_imp = PhysicsEngine::makeVec3(angular);

		(this->m_body_interface)->AddImpulse(jolt_id, angular_imp);
		(this->m_body_interface)->AddAngularImpulse(jolt_id, angular_imp);
	}

	void PhysicsEngine::OptimizeBroadPhase()
	{
		(this->m_physics_system)->OptimizeBroadPhase();
		s_JoltData->hasOptimized = true;
	}

	void PhysicsEngine::Step(float deltaTime)
	{
		if (!s_JoltData->hasOptimized)
		{
			this->OptimizeBroadPhase();
		}

		(this->m_physics_system)->Update(
			deltaTime,
			s_JoltData->collisionSteps,
			s_JoltData->integrationSubSteps,
			s_JoltData->TemporariesAllocator,
			s_JoltData->JobThreadPool.get()
		);

		// TODO: Add post simulation step 
		// character.PostSimulation();
	}

	void PhysicsEngine::RemoveShape(UUID entity_id)
	{
		JPH::BodyID h_id = HBodyID::GetBodyID(entity_id);
		(this->m_body_interface)->RemoveBody(h_id);
	}

	void PhysicsEngine::DestroyShape(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		(this->m_body_interface)->DestroyBody(jolt_id);
	}

	void PhysicsEngine::DestroyAllShapes()
	{
		auto& body_id_map = HBodyID::GetMap();
		for (auto& [entity_id, jolt_id] : body_id_map)
		{
			(this->m_body_interface)->RemoveBody(jolt_id);
			(this->m_body_interface)->DestroyBody(jolt_id);
		}		
		s_JoltData->numberOfBodies = 0;
		HBodyID::EmptyMap();
	}

	bool PhysicsEngine::IsActive(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		return (this->m_body_interface)->IsActive(jolt_id);
	}

	bool PhysicsEngine::HasCollider(UUID entity_id)
	{
		auto& entity_map = HBodyID::GetMap();

		return entity_map.find(entity_id) != entity_map.end();
	}

	glm::vec3 PhysicsEngine::GetCenterOfMassPosition(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::vec3(0.f);
		}

		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RVec3 vec = (this->m_body_interface)->GetCenterOfMassPosition(jolt_id);
		return PhysicsEngine::makeGLMVec3(vec);
	}

	glm::vec3 PhysicsEngine::GetPosition(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::vec3(0.f);
		}

		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RVec3 vec = (this->m_body_interface)->GetPosition(jolt_id);
		return PhysicsEngine::makeGLMVec3(vec);
	}

	glm::mat4x4 PhysicsEngine::GetCenterOfMassTransform(UUID id)
	{
		if (!HasCollider(id)) 
		{
			return glm::mat4(); // Returns identity
		}
		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RMat44 vec = (this->m_body_interface)->GetCenterOfMassTransform(jolt_id);
		return PhysicsEngine::makeMat4x4(vec);
	}

	glm::mat4 PhysicsEngine::GetTransform(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::mat4(); // Returns identity
		}
		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RMat44 vec = (this->m_body_interface)->GetWorldTransform(jolt_id);
		return PhysicsEngine::makeMat4x4(vec);
	}

	glm::vec3 PhysicsEngine::GetLinearVelocity(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		JPH::Vec3 vec = (this->m_body_interface)->GetLinearVelocity(jolt_id);
		return PhysicsEngine::makeGLMVec3(vec);
	}

	glm::vec3 PhysicsEngine::GetAngularVelocity(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		JPH::Vec3 vec = (this->m_body_interface)->GetAngularVelocity(jolt_id);
		return PhysicsEngine::makeGLMVec3(vec);
	}

	void PhysicsEngine::OnRuntimeStart(int collisionSteps, int integrationSubStep)
	{
		s_JoltData->collisionSteps = collisionSteps;
		s_JoltData->integrationSubSteps = integrationSubStep;
	}

	void PhysicsEngine::OnRuntimeStop()
	{
		s_JoltData->hasOptimized = false;
		this->DestroyAllShapes();
	}
}
