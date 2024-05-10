#include "pch.h"
#include "PhysicsScene.h"

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>


namespace Engine {

	static BPLayerInterfaceImpl s_broad_phase_layer_interface;
	static ObjectVsBroadPhaseLayerFilterImpl s_object_vs_broadphase_layer_filter;
	static ObjectLayerPairFilterImpl s_object_vs_object_layer_filter;

	PhysicsScene::PhysicsScene(Scene* scene, JPH::TempAllocator* temporariesAllocator = nullptr, JPH::JobSystemThreadPool* jobThreadPool = nullptr) : m_scene(scene)
	{
		if (s_temporariesAllocator == nullptr || s_jobThreadPool == nullptr)
		{
			s_temporariesAllocator = temporariesAllocator;
			s_jobThreadPool = jobThreadPool;
		}

		m_physics_system = CreateRef<JPH::PhysicsSystem>();
		(this->m_physics_system)->Init(
			cMaxBodies,
			cNumBodyMutexes,
			cMaxBodyPairs,
			cMaxContactConstraints,
			s_broad_phase_layer_interface,
			s_object_vs_broadphase_layer_filter,
			s_object_vs_object_layer_filter
		);

		m_activation_listener = CreateRef<MyBodyActivationListener>();
		m_contact_listener = CreateRef<MyContactListener>();
		m_contact_listener->SetCurrentScene(this);

		m_physics_system->SetContactListener(m_contact_listener.get());

		m_physics_system->SetBodyActivationListener(m_activation_listener.get());
		
		this->m_body_interface = &(m_physics_system->GetBodyInterface());

		gravity = PhysicsScene::makeGLMVec3(m_physics_system->GetGravity());
	}

	glm::vec3 PhysicsScene::GetGravity()
	{
		return gravity;
	}

	void PhysicsScene::SetGravity(glm::vec3 gravity)
	{
		this->gravity = gravity;
		this->m_physics_system->SetGravity(PhysicsScene::makeVec3(this->gravity));
	}

	void PhysicsScene::Update(float deltaTime)
	{
		this->m_newContact.clear();
		this->m_persistContact.clear();
		this->m_removedContact.clear();

		if (!s_hasOptimized)
		{
			this->OptimizeBroadPhase();
		}

		(this->m_physics_system)->Update(
			deltaTime,
			collisionSteps,
			integrationSubSteps,
			s_temporariesAllocator,
			s_jobThreadPool
		);


		// post simulation
		for (auto& [entity_id, character] : m_characterMap)
		{
			character->PostSimulation(0.0f);		// TODO: check if this works on removed but undeleted characters
		}

		for (auto& [id1, id2] : this->m_newContact)
		{
			ScriptEngine::CallMethod<uint64_t>(id1, "newCollision", id2);
			ScriptEngine::CallMethod<uint64_t>(id2, "newCollision", id1);
		}

		for (auto& [id1, id2] : this->m_persistContact)
		{
			ScriptEngine::CallMethod<uint64_t>(id1, "persistCollision", id2);
			ScriptEngine::CallMethod<uint64_t>(id2, "persistCollision", id1);
		}

		for (auto& [id1, id2] : this->m_removedContact)
		{
			ScriptEngine::CallMethod<uint64_t>(id1, "removedCollision", id2);
			ScriptEngine::CallMethod<uint64_t>(id2, "removedCollision", id1);
		}
	}

	std::vector<HBodyID> PhysicsScene::CreateBody(Entity* entity)
	{
		std::vector<HBodyID> res;

		BoxColliderComponent* boxComponent = entity->GetComponent<BoxColliderComponent>();
		if (boxComponent)
		{
			glm::vec3 position = entity->GetComponent<TransformComponent>()->world_transform.translation;
			glm::vec3 scale = entity->GetComponent<TransformComponent>()->world_transform.scale;

			glm::vec3 dimensions = glm::vec3(boxComponent->HalfSize.x * scale.x, boxComponent->HalfSize.y * scale.y, boxComponent->HalfSize.z * scale.z);

			res.push_back(this->CreateBox(
				entity->GetID(),
				dimensions,
				position,
				boxComponent->MotionType,
				boxComponent->Offset,
				true
			));
		}
		SphereColliderComponent* sphereComponent = entity->GetComponent<SphereColliderComponent>();
		if (sphereComponent)
		{
			glm::vec3 position = entity->GetComponent<TransformComponent>()->world_transform.translation;
			res.push_back(this->CreateSphere(
				entity->GetID(),
				sphereComponent->Radius,
				position,
				sphereComponent->MotionType,
				sphereComponent->Offset,
				true
			));
		}
		CharacterControllerComponent* characterComponent = entity->GetComponent <CharacterControllerComponent>(); {}
		if (characterComponent)
		{
			glm::vec3 position = entity->GetComponent<TransformComponent>()->world_transform.translation;
			res.push_back(this->CreateCharacter(
				entity->GetID(),
				characterComponent->Mass,
				characterComponent->HalfHeight,
				characterComponent->Radius,
				position,
				characterComponent->Offset,
				characterComponent->UserData
			));
		}
		return res;
	}

	HBodyID PhysicsScene::CreateBox(UUID entity_id, glm::vec3 dimensions, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate)
	{
		// conversion
		JPH::Vec3 dim = PhysicsScene::makeVec3(dimensions);
		JPH::RVec3 pos = PhysicsScene::makeRVec3(position);
		JPH::EMotionType mov = PhysicsScene::makeEMotionType(movability);
		JPH::Vec3 jolt_offset = PhysicsScene::makeVec3(offset);

		JPH::BoxShapeSettings* box_shape_settings = new JPH::BoxShapeSettings(dim);		// TODO: I think we can add material here
		JPH::RotatedTranslatedShapeSettings offsetShapeSettings(jolt_offset, JPH::Quat::sIdentity(), box_shape_settings);

		JPH::ShapeSettings::ShapeResult box_shape_result = offsetShapeSettings.Create();
		if (box_shape_result.HasError())
		{
			this->m_LastErrorMessage = box_shape_result.GetError();
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

		JPH::Body* box_body = (this->m_body_interface)->CreateBody(box_settings);
		box_body->SetUserData(entity_id);
		// Add it to the world
		if (activate)
		{
			(this->m_body_interface)->AddBody(box_body->GetID(), JPH::EActivation::Activate);
			//box_id = (this->m_body_interface)->CreateAndAddBody(box_settings, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->AddBody(box_body->GetID(), JPH::EActivation::DontActivate);
			//box_id = (this->m_body_interface)->CreateAndAddBody(box_settings, JPH::EActivation::DontActivate);
		}

		box_shape_settings->Release();

		//this->m_numberOfBodies++;
		this->s_hasOptimized = false;
		this->m_bodyMap[entity_id] = box_body;

		return HBodyID(entity_id, box_body->GetID());
	}

	HBodyID PhysicsScene::CreateSphere(UUID entity_id, float radius, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate)
	{
		JPH::RVec3 pos = PhysicsScene::makeRVec3(position);
		JPH::EMotionType mov = PhysicsScene::makeEMotionType(movability);
		JPH::Vec3 jolt_offset = PhysicsScene::makeVec3(offset);

		JPH::SphereShapeSettings* sphere_shape_settings = new JPH::SphereShapeSettings(radius);		// TODO: I think we can add material here
		JPH::RotatedTranslatedShapeSettings offsetShapeSettings(jolt_offset, JPH::Quat::sIdentity(), sphere_shape_settings);
		JPH::ShapeSettings::ShapeResult sphere_shape_result = offsetShapeSettings.Create();
		if (sphere_shape_result.HasError())
		{
			this->m_LastErrorMessage = sphere_shape_result.GetError();
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
		JPH::Body* sphere_body = (this->m_body_interface)->CreateBody(sphere_settings);
		sphere_body->SetUserData(entity_id);
		if (activate)
		{
			(this->m_body_interface)->AddBody(sphere_body->GetID(), JPH::EActivation::Activate);
			//sphere_id = (this->m_body_interface)->CreateAndAddBody(sphere_settings, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->AddBody(sphere_body->GetID(), JPH::EActivation::DontActivate);
			//sphere_id = (this->m_body_interface)->CreateAndAddBody(sphere_settings, JPH::EActivation::DontActivate);
		}

		sphere_shape_settings->Release();

		//this->m_numberOfBodies++;
		this->s_hasOptimized = false;
		this->m_bodyMap[entity_id] = sphere_body;

		return HBodyID(entity_id, sphere_body->GetID());
	}

	HBodyID PhysicsScene::CreateCharacter(UUID entity_id, float mass, float halfHeight, float radius, glm::vec3 position, glm::vec3 offset, std::uint64_t userData)
	{
		Scope<JPH::CharacterSettings> character_settings = CreateScope<JPH::CharacterSettings>();
		//character_settings->mMass = mass;
		//character_settings->mMaxStrength = strength;
		
		JPH::RVec3 jolt_offset = PhysicsScene::makeRVec3(offset);
		JPH::CapsuleShapeSettings* capShapeSettings = new JPH::CapsuleShapeSettings(halfHeight, radius);		// TODO: add material
		JPH::ShapeSettings::ShapeResult capsuleResult = capShapeSettings->Create();
		
		JPH::RotatedTranslatedShapeSettings capsuleTransSettings(jolt_offset, JPH::Quat::sIdentity(), capsuleResult.Get());
		JPH::Shape* capsule = capsuleTransSettings.Create().Get();
		capsule->SetUserData(entity_id);
		
		character_settings->mShape = capsule;
		JPH::Character* character = new JPH::Character(
			character_settings.get(),
			PhysicsScene::makeRVec3(position),
			JPH::Quat::sIdentity(),
			userData,
			this->m_physics_system.get()
		);
		
		character->SetShape(capsule, FLT_MAX);
		character->SetLayer(Layers::MOVING);
		character->AddToPhysicsSystem();

		delete capShapeSettings;

		m_characterMap[entity_id] = character;
		return HBodyID(entity_id, character->GetBodyID());
	}

	void PhysicsScene::InsertObjectByID(UUID entity_id, bool activate)
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

	void PhysicsScene::SetPosition(UUID entity_id, glm::vec3 position, bool activate)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		JPH::RVec3 pos = PhysicsScene::makeRVec3(position);
		if (activate)
		{
			(this->m_body_interface)->SetPosition(jolt_id, pos, JPH::EActivation::Activate);
		}
		else
		{
			(this->m_body_interface)->SetPosition(jolt_id, pos, JPH::EActivation::DontActivate);
		}
	}

	void PhysicsScene::SetLinearVelocity(UUID entity_id, glm::vec3& velocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 vel = PhysicsScene::makeVec3(velocity);

		(this->m_body_interface)->SetLinearVelocity(jolt_id, vel);
	}

	void PhysicsScene::SetAngularVelocity(UUID entity_id, glm::vec3& velocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 vel = PhysicsScene::makeVec3(velocity);

		(this->m_body_interface)->SetAngularVelocity(jolt_id, vel);
	}

	void PhysicsScene::SetLinearAndAngularVelocity(UUID entity_id, glm::vec3& linaerVelocity, glm::vec3& angularVelocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 lVel = PhysicsScene::makeVec3(linaerVelocity);
		JPH::Vec3 aVel = PhysicsScene::makeVec3(angularVelocity);

		(this->m_body_interface)->SetLinearAndAngularVelocity(jolt_id, lVel, aVel);
	}

	void PhysicsScene::AddLinearVelocity(UUID entity_id, glm::vec3& velocity)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 vel = PhysicsScene::makeVec3(velocity);

		(this->m_body_interface)->AddLinearVelocity(jolt_id, vel);
	}

	void PhysicsScene::AddLinearImpulse(UUID entity_id, glm::vec3& impulse)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 imp = PhysicsScene::makeVec3(impulse);

		(this->m_body_interface)->AddImpulse(jolt_id, imp);
	}

	void PhysicsScene::AddAngularImpulse(UUID entity_id, glm::vec3& impulse)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 imp = PhysicsScene::makeVec3(impulse);

		(this->m_body_interface)->AddAngularImpulse(jolt_id, imp);
	}

	void PhysicsScene::AddLinearAndAngularImpulse(UUID entity_id, glm::vec3& linear, glm::vec3& angular)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);
		JPH::Vec3 linear_imp = PhysicsScene::makeVec3(linear);
		JPH::Vec3 angular_imp = PhysicsScene::makeVec3(angular);

		(this->m_body_interface)->AddImpulse(jolt_id, angular_imp);
		(this->m_body_interface)->AddAngularImpulse(jolt_id, angular_imp);
	}

	bool PhysicsScene::IsOptimized()
	{
		return s_hasOptimized;
	}

	void PhysicsScene::SetOptimized(bool optimized)
	{
		s_hasOptimized = optimized;
	}

	void PhysicsScene::OptimizeBroadPhase()
	{
		this->m_physics_system->OptimizeBroadPhase();
	}

	void PhysicsScene::RemoveShape(UUID entity_id)
	{
		JPH::BodyID h_id = HBodyID::GetBodyID(entity_id);
		(this->m_body_interface)->RemoveBody(h_id);
	}

	void PhysicsScene::DestroyShape(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		(this->m_body_interface)->DestroyBody(jolt_id);
		HBodyID::RemoveEntry(entity_id);
		m_bodyMap.erase(entity_id);
	}

	void PhysicsScene::DestroyAllShapes()
	{
		std::vector<UUID> toKill = std::vector<UUID>();
		for (auto& [entity_id, body] : m_bodyMap)
		{
			(this->m_body_interface)->RemoveBody(body->GetID());
			(this->m_body_interface)->DestroyBody(body->GetID());
			HBodyID::RemoveEntry(entity_id);
			toKill.push_back(entity_id);
		}
		for (auto& entity_id : toKill)
		{
			m_bodyMap.erase(entity_id);
		}
		//HBodyID::EmptyMap();
	}

	void PhysicsScene::RemoveCharacter(UUID entity_id)
	{
		m_characterMap[entity_id]->RemoveFromPhysicsSystem();
	}

	void PhysicsScene::DestroyCharacter(UUID entity_id)
	{
		this->RemoveCharacter(entity_id);
		delete m_characterMap[entity_id];
		m_characterMap.erase(entity_id);
		HBodyID::RemoveEntry(entity_id);
	}

	void PhysicsScene::DestroyAllCharacters()
	{
		std::vector<UUID> toKill = std::vector<UUID>();
		for (auto& [entity_id, character] : m_characterMap)
		{
			toKill.push_back(entity_id);
			character->RemoveFromPhysicsSystem();
		}
		for (auto& entity_id : toKill)
		{
			delete m_characterMap[entity_id];
			m_characterMap.erase(entity_id);
			HBodyID::RemoveEntry(entity_id);
		}
	}

	void PhysicsScene::DestroyAll()
	{
		DestroyAllShapes();
		DestroyAllCharacters();
	}


	bool PhysicsScene::IsActive(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		return (this->m_body_interface)->IsActive(jolt_id);
	}

	bool PhysicsScene::IsActive(HBodyID h_id)
	{
		JPH::BodyID jolt_id = h_id.GetBodyID();

		return (this->m_body_interface)->IsActive(jolt_id);
	}

	bool PhysicsScene::HasCollider(UUID entity_id)
	{
		auto& entity_map = HBodyID::GetMap();

		return entity_map.find(entity_id) != entity_map.end();
	}

	void PhysicsScene::SetCollisionAndIntegrationSteps(int collisionSteps, int integrationSubSteps)
	{
		if (collisionSteps != 0)
		{
			this->collisionSteps = collisionSteps;
		}
		if (integrationSubSteps != 0)
		{
			this->integrationSubSteps = integrationSubSteps;
		}
	}

	void PhysicsScene::AddNewContact(UUID id1, UUID id2)
	{
		this->m_newContact.push_back(std::pair<UUID, UUID>(id1, id2));
	}

	void PhysicsScene::AddPersistContact(UUID id1, UUID id2)
	{
		this->m_persistContact.push_back(std::pair<UUID, UUID>(id1, id2));
	}

	void PhysicsScene::AddRemoveContact(UUID id1, UUID id2)
	{
		this->m_removedContact.push_back(std::pair<UUID, UUID>(id1, id2));
	}

	std::vector<std::pair<UUID, UUID>> PhysicsScene::GetNewContacts()
	{
		return this->m_newContact;
	}

	std::vector<std::pair<UUID, UUID>> PhysicsScene::GetPersistContacts()
	{
		return this->m_persistContact;
	}

	UUID PhysicsScene::GetUserData(JPH::BodyID id)
	{
		return this->m_body_interface->GetShape(id)->GetUserData();
	}

	std::vector<std::pair<UUID, UUID>> PhysicsScene::GetRemovedContacts()
	{
		return this->m_removedContact;
	}

	glm::vec3 PhysicsScene::GetCenterOfMassPosition(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::vec3(0.f);
		}

		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RVec3 vec = (this->m_body_interface)->GetCenterOfMassPosition(jolt_id);
		return PhysicsScene::makeGLMVec3(vec);
	}

	glm::vec3 PhysicsScene::GetPosition(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::vec3(0.f);
		}

		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RVec3 vec = (this->m_body_interface)->GetPosition(jolt_id);
		return PhysicsScene::makeGLMVec3(vec);
	}

	glm::mat4x4 PhysicsScene::GetCenterOfMassTransform(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::mat4(); // Returns identity
		}
		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RMat44 vec = (this->m_body_interface)->GetCenterOfMassTransform(jolt_id);
		return PhysicsScene::makeMat4x4(vec);
	}

	glm::mat4 PhysicsScene::GetTransform(UUID id)
	{
		if (!HasCollider(id))
		{
			return glm::mat4(); // Returns identity
		}
		JPH::BodyID jolt_id = HBodyID::GetBodyID(id);

		JPH::RMat44 vec = (this->m_body_interface)->GetWorldTransform(jolt_id);
		return PhysicsScene::makeMat4x4(vec);
	}

	glm::vec3 PhysicsScene::GetLinearVelocity(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		JPH::Vec3 vec = (this->m_body_interface)->GetLinearVelocity(jolt_id);
		return PhysicsScene::makeGLMVec3(vec);
	}

	glm::vec3 PhysicsScene::GetAngularVelocity(UUID entity_id)
	{
		JPH::BodyID jolt_id = HBodyID::GetBodyID(entity_id);

		JPH::Vec3 vec = (this->m_body_interface)->GetAngularVelocity(jolt_id);
		return PhysicsScene::makeGLMVec3(vec);
	}



	JPH::RVec3 PhysicsScene::makeRVec3(glm::vec3 arr)
	{
		return JPH::RVec3(arr.x, arr.y, arr.z);
	}
	JPH::Vec3 PhysicsScene::makeVec3(glm::vec3 arr)
	{
		return JPH::Vec3(arr.x, arr.y, arr.z);
	}
	glm::vec3 PhysicsScene::makeGLMVec3(JPH::Vec3 arr)
	{
		return glm::vec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
	glm::vec4 PhysicsScene::makeGLMVec4(JPH::Vec4 arr)
	{
		return glm::vec4(arr.GetX(), arr.GetY(), arr.GetZ(), arr.GetW());
	}
	glm::mat4x4 PhysicsScene::makeMat4x4(JPH::RMat44 mat)
	{
		return glm::mat4x4{
			PhysicsScene::makeGLMVec4(mat.GetColumn4(0)),
			PhysicsScene::makeGLMVec4(mat.GetColumn4(1)),
			PhysicsScene::makeGLMVec4(mat.GetColumn4(2)),
			PhysicsScene::makeGLMVec4(mat.GetColumn4(3))
		};
	}
	//HVec3 PhysicsScene::makeHVec3(JPH::RVec3 arr)
	//{
	//	return HVec3(arr.GetX(), arr.GetY(), arr.GetZ());
	//}
	JPH::EMotionType PhysicsScene::makeEMotionType(HEMotionType movability)
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
	HEMotionType PhysicsScene::makeHEMotionType(JPH::EMotionType movability)
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
}
