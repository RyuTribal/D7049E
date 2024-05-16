#pragma once

#ifndef HPhysicsScene_h
#define HPhysicsScene_h

#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
//#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Character/Character.h>

#include "Auxiliary/HVec3.h"
#include "Auxiliary/HVec3.h"
#include "Auxiliary/HEMotionType.h"
#include "Auxiliary/PhysicsIDManager.h"
#include "Auxiliary/Layers.h"

#include "Scene/Entity.h"
#include "Scene/Components.h"

namespace Engine {
	
	class BPLayerInterfaceImpl final : public JPH::BroadPhaseLayerInterface
	{
	public:
		BPLayerInterfaceImpl()
		{
			// Create a mapping table from object to broad phase layer
			mObjectToBroadPhase[Layers::NON_MOVING] = BroadPhaseLayers::NON_MOVING;
			mObjectToBroadPhase[Layers::MOVING] = BroadPhaseLayers::MOVING;
		}

		virtual JPH::uint					GetNumBroadPhaseLayers() const override
		{
			return BroadPhaseLayers::NUM_LAYERS;
		}

		virtual JPH::BroadPhaseLayer			GetBroadPhaseLayer(JPH::ObjectLayer inLayer) const override
		{
			//JPH_ASSERT(inLayer < Layers::NUM_LAYERS);
			HVE_ASSERT(inLayer < Layers::NUM_LAYERS);
			return mObjectToBroadPhase[inLayer];
		}

	private:
		JPH::BroadPhaseLayer					mObjectToBroadPhase[Layers::NUM_LAYERS];
	};

	/// Class that determines if an object layer can collide with a broadphase layer
	class ObjectVsBroadPhaseLayerFilterImpl : public JPH::ObjectVsBroadPhaseLayerFilter
	{
	public:
		virtual bool				ShouldCollide(JPH::ObjectLayer inLayer1, JPH::BroadPhaseLayer inLayer2) const override
		{
			switch (inLayer1)
			{
				case Layers::NON_MOVING:
					return inLayer2 == BroadPhaseLayers::MOVING;
				case Layers::MOVING:
					return true;
				default:
					//JPH_ASSERT(false);
					HVE_ASSERT(false);
					return false;
			}
		}
	};

	/// Class that determines if two object layers can collide
	class ObjectLayerPairFilterImpl : public JPH::ObjectLayerPairFilter
	{
	public:
		virtual bool					ShouldCollide(JPH::ObjectLayer inObject1, JPH::ObjectLayer inObject2) const override
		{
			switch (inObject1)
			{
				case Layers::NON_MOVING:
					return inObject2 == Layers::MOVING; // Non moving only collides with moving
				case Layers::MOVING:
					return true; // Moving collides with everything
				default:
					//JPH_ASSERT(false);
					HVE_ASSERT(false);
					return false;
			}
		}
	};

	// An example activation listener
	class MyBodyActivationListener : public JPH::BodyActivationListener
	{
	public:
		virtual void		OnBodyActivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			std::cout << "Body activation" << std::endl;
			HVE_CORE_TRACE("A body got activated");
		}

		virtual void		OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			std::cout << "Body sleep" << std::endl;
			HVE_CORE_TRACE("A body went to sleep");
		}
	};

	// An example contact listener
	//#ifndef HContactListener_h
	class HContactListener;
	//#endif

	class HPhysicsScene
	{
	public: 
		HPhysicsScene(Scene* scene, JPH::TempAllocator* temporariesAllocator, JPH::JobSystemThreadPool* jobThreadPool);
		glm::vec3 GetGravity();
		void SetGravity(glm::vec3);
		void Update(float deltaTime);

		std::vector<HBodyID> CreateBody(Entity* entity);
		HBodyID CreateBox(UUID entity_id, float mass, glm::vec3 dimensions, glm::quat rotation, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate, float friction, float restitution);
		HBodyID CreateSphere(UUID entity_id, float mass, float radius, glm::vec3 position, glm::quat rotation, HEMotionType movability, glm::vec3& offset, bool activate, float friction, float restitution);
		HBodyID CreateCharacter(UUID entity_id, float mass, float halfHeight, float radius, glm::vec3 position, glm::quat rotation, glm::vec3 offset, float friction, float restitution);
		void InsertObjectByID(UUID entity_id, bool activate);
		void SetPosition(UUID entity_id, glm::vec3 position, bool activate);
		void SetLinearVelocity(UUID entity_id, glm::vec3& velocity);
		void SetAngularVelocity(UUID entity_id, glm::vec3& velocity);
		void SetLinearAndAngularVelocity(UUID entity_id, glm::vec3& linaerVelocity, glm::vec3& angularVelocity);
		void AddLinearVelocity(UUID entity_id, glm::vec3& velocity);

		void AddLinearImpulse(UUID entity_id, glm::vec3& impulse);
		void AddAngularImpulse(UUID entity_id, glm::vec3& impulse);
		void AddLinearAndAngularImpulse(UUID entity_id, glm::vec3& linear, glm::vec3& angular);
		glm::vec3 GetRotation(UUID entity_id);
		void SetRotation(UUID entity_id, glm::vec3& rotation);
		void Rotate(UUID entity_id, glm::vec3& delta);

		bool IsOptimized();
		void SetOptimized(bool optimized);
		void OptimizeBroadPhase();
		void RemoveShape(UUID entity_id);
		void DestroyShape(UUID entity_id);
		void DestroyAllShapes();
		void RemoveCharacter(UUID entity_id);
		void DestroyCharacter(UUID entity_id);
		bool IsCharacterGrounded(UUID entity_id);
		void DestroyAllCharacters();
		void DestroyAll();
		bool IsActive(UUID entity_id);
		bool IsActive(HBodyID h_id);
		bool HasCollider(UUID entity_id);
		void SetCollisionAndIntegrationSteps(int collisionSteps, int integrationSubSteps);

		UUID GetUserData(JPH::BodyID id);	// This is actually internal, do not use

		void AddNewContact(UUID id1, UUID id2);
		void AddPersistContact(UUID id1, UUID id2);
		void AddRemoveContact(UUID id1, UUID id2);
		std::vector<std::pair<UUID, UUID>> GetNewContacts();
		std::vector<std::pair<UUID, UUID>> GetPersistContacts();
		std::vector<std::pair<UUID, UUID>> GetRemovedContacts();

		glm::vec3 GetCenterOfMassPosition(UUID id);
		glm::vec3 GetPosition(UUID id);
		glm::mat4x4 GetCenterOfMassTransform(UUID id);
		glm::mat4 GetTransform(UUID id);
		glm::vec3 GetLinearVelocity(UUID entity_id);
		glm::vec3 GetAngularVelocity(UUID entity_id);


		Ref<JPH::PhysicsSystem> GetSystem() { return m_physics_system; }

	private:

		Scene* m_scene;

		std::string m_LastErrorMessage = "";
		Ref<JPH::PhysicsSystem> m_physics_system;
		JPH::BodyInterface* m_body_interface;
		Ref<HContactListener> m_contact_listener;
		Ref<MyBodyActivationListener> m_activation_listener;

		inline static JPH::TempAllocator* s_temporariesAllocator = nullptr;
		inline static JPH::JobSystemThreadPool* s_jobThreadPool = nullptr;

		std::string LastErrorMessage = "";

		JPH::uint cMaxBodies = 65536;
		JPH::uint cNumBodyMutexes = 0;
		JPH::uint cMaxBodyPairs = 65536;
		JPH::uint cMaxContactConstraints = 10240;
		//float cDeltaTime = 1.0f / 60.0f;	// 60 Hz
		int collisionSteps = 1;
		int integrationSubSteps = 1;

		glm::vec3 gravity;

		inline static bool s_hasOptimized = false;

		std::map<UUID, JPH::Body*> m_bodyMap = std::map<UUID, JPH::Body*>();
		std::map<UUID, JPH::Character*> m_characterMap = std::map<UUID, JPH::Character*>();
		std::vector<std::pair<UUID, UUID>> m_newContact = std::vector<std::pair<UUID, UUID>>();
		std::vector<std::pair<UUID, UUID>> m_persistContact = std::vector<std::pair<UUID, UUID>>();
		std::vector<std::pair<UUID, UUID>> m_removedContact = std::vector<std::pair<UUID, UUID>>();

		static JPH::RVec3 makeRVec3(glm::vec3 arr);
		static JPH::Vec3 makeVec3(glm::vec3 arr);
		static glm::vec3 makeGLMVec3(JPH::Vec3 arr);
		static glm::vec4 makeGLMVec4(JPH::Vec4 arr);
		static glm::mat4x4 makeMat4x4(JPH::RMat44 mat);
		//static HVec3 makeHVec3(JPH::RVec3 arr);
		static JPH::EMotionType makeEMotionType(HEMotionType movability);
		static HEMotionType makeHEMotionType(JPH::EMotionType movability);

	};
	
}
#endif


