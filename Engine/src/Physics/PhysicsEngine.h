#pragma once
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/RegisterTypes.h>
#include <Jolt/Core/Factory.h>
#include <Jolt/Physics/PhysicsSettings.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>

#include "Auxiliary/HVec3.h"
#include "Auxiliary/HVec3.h"
#include "Auxiliary/HEMotionType.h"
#include "Auxiliary/HBodyID.h"


namespace Engine {

	namespace BroadPhaseLayers {	// TODO: structure this a bit better. Own file perhaps
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS(2);
	};

	namespace Layers {
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING = 1;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	};

	// BroadPhaseLayerInterface implementation
	// This defines a mapping between object and broadphase layers.
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
			std::cout << "A body got activated" << std::endl;
		}

		virtual void		OnBodyDeactivated(const JPH::BodyID& inBodyID, JPH::uint64 inBodyUserData) override
		{
			std::cout << "A body went to sleep" << std::endl;
		}
	};

	// An example contact listener
	class MyContactListener : public JPH::ContactListener
	{
	public:
		// See: ContactListener
		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override
		{
			std::cout << "Contact validate callback" << std::endl;

			// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
			return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
		}

		virtual void			OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			std::cout << "A contact was added" << std::endl;
		}

		virtual void			OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override
		{
			std::cout << "A contact was persisted" << std::endl;
		}

		virtual void			OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override
		{
			std::cout << "A contact was removed" << std::endl;
		}
	};


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
		//HBodyID createBox(JPH::Vec3 dimensions, JPH::RVec3 position, JPH::EMotionType movability, bool activate);		// TODO: change Vec3 to normal vector
		HBodyID CreateBox(HVec3 dimensions, HVec3 position, HEMotionType movability, bool activate);		// TODO: change Vec3 to normal vector
		HBodyID CreateSphere(float radius, HVec3 position, HEMotionType movability, bool activate);
		void InsertObjectByID(HBodyID id, bool activate);
		void SetPosition(HBodyID id, HVec3 position, bool activate);
		void SetLinearVelocity(HBodyID id, HVec3 velocity);
		void SetAngularVelocity(HBodyID id, HVec3 velocity);
		void SetLinearAndAngularVelocity(HBodyID id, HVec3 linaerVelocity, HVec3 angularVelocity);
		void OptimizeBroadPhase();
		void Step(float deltaTime);
		void RemoveBody(HBodyID id);
		void DestoryBody(HBodyID id);
		void DestoryAllBodies();
		bool IsActive(HBodyID id);
		HVec3 GetCenterOfMassPosition(HBodyID id);
		HVec3 GetLinearVelocity(HBodyID id);
		void OnRuntimeStart(int collisionSteps, int integrationSubStep);
		void OnRuntimeStop();
		static void tmpRunner();
		

	private:
		static PhysicsEngine* s_Instance;

		//JPH::TempAllocatorImpl* m_temp_allocator;
		//JPH::JobSystemThreadPool* m_job_system;
		Ref<JPH::PhysicsSystem> m_physics_system;



		BPLayerInterfaceImpl m_broad_phase_layer_interface;
		ObjectVsBroadPhaseLayerFilterImpl m_object_vs_broadphase_layer_filter;
		ObjectLayerPairFilterImpl m_object_vs_object_layer_filter;

		MyBodyActivationListener m_body_activation_listener;

		MyContactListener m_contact_listener;

		JPH::BodyInterface* m_body_interface;

		static JPH::RVec3 makeRVec3(HVec3 arr);
		static JPH::Vec3 makeVec3(HVec3 arr);
		static HVec3 makeHVec3(JPH::Vec3 arr);
		//static HVec3 makeHVec3(JPH::RVec3 arr);
		static JPH::EMotionType makeEMotionType(HEMotionType movability);
		static HEMotionType makeHEMotionType(JPH::EMotionType movability);


	};

}

