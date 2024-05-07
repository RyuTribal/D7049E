#pragma once
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
			HVE_CORE_TRACE("A body got activated");
		}

		virtual void		OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			HVE_CORE_TRACE("A body went to sleep");
		}
	};

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
		{
			HVE_CORE_TRACE("Contact validate callback");

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void			OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			HVE_CORE_TRACE("A contact was added");
		}

		virtual void			OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			HVE_CORE_TRACE("A contact was persisted");
		}

		virtual void			OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			HVE_CORE_TRACE("A contact was removed");
		}
	};


	class PhysicsScene
	{
	public: 
		PhysicsScene(int allocationSize);
		glm::vec3 GetGravity();
		void SetGravity(glm::vec3);
		void Update();

		std::vector<HBodyID> CreateBody(Entity* entity);
		HBodyID CreateBox(UUID entity_id, glm::vec3 dimensions, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate);
		HBodyID CreateSphere(UUID entity_id, float radius, glm::vec3 position, HEMotionType movability, glm::vec3& offset, bool activate);
		HBodyID CreateCharacter(UUID entity_id, float mass, float halfHeight, float radius, glm::vec3 position, glm::vec3 offset, std::uint64_t userData);
		void InsertObjectByID(UUID entity_id, bool activate);
		void SetPosition(UUID entity_id, glm::vec3 position, bool activate);
		void SetLinearVelocity(UUID entity_id, glm::vec3& velocity);
		void SetAngularVelocity(UUID entity_id, glm::vec3& velocity);
		void SetLinearAndAngularVelocity(UUID entity_id, glm::vec3& linaerVelocity, glm::vec3& angularVelocity);
		void AddLinearVelocity(UUID entity_id, glm::vec3& velocity);

		void AddLinearImpulse(UUID entity_id, glm::vec3& impulse);
		void AddAngularImpulse(UUID entity_id, glm::vec3& impulse);
		void AddLinearAndAngularImpulse(UUID entity_id, glm::vec3& linear, glm::vec3& angular);

		bool IsOptimized();
		void SetOptimized(bool optimized);
		void OptimizeBroadPhase();
		void RemoveShape(UUID entity_id);
		void DestroyShape(UUID entity_id);
		void DestroyAllShapes();
		bool IsActive(UUID entity_id);
		bool HasCollider(UUID entity_id);
		glm::vec3 GetCenterOfMassPosition(UUID id);
		glm::vec3 GetPosition(UUID id);
		glm::mat4x4 GetCenterOfMassTransform(UUID id);
		glm::mat4 GetTransform(UUID id);
		glm::vec3 GetLinearVelocity(UUID entity_id);
		glm::vec3 GetAngularVelocity(UUID entity_id);

	private:

		std::string m_LastErrorMessage = "";
		Ref<JPH::PhysicsSystem> m_physics_system;
		JPH::BodyInterface* m_body_interface;
		Ref<MyContactListener> m_contact_listener;

		static JPH::TempAllocator* s_temporariesAllocator;
		static std::unique_ptr<JPH::JobSystemThreadPool> s_jobThreadPool;
		static BPLayerInterfaceImpl s_broad_phase_layer_interface;
		static ObjectVsBroadPhaseLayerFilterImpl s_object_vs_broadphase_layer_filter;
		static ObjectLayerPairFilterImpl s_object_vs_object_layer_filter;

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

		std::map<HBodyID, JPH::Body*> m_bodyMap = std::map<HBodyID, JPH::Body*>();

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


