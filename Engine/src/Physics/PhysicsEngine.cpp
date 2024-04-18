#include "pch.h"
#include "PhysicsEngine.h"

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
	HVec3 PhysicsEngine::makeHVec3(JPH::RVec3 arr)
	{
		return HVec3(arr.GetX(), arr.GetY(), arr.GetZ());
	}
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

		s_JoltData->TemporariesAllocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		s_JoltData->JobThreadPool = std::unique_ptr<JPH::JobSystemThreadPool> ( 
			new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1)
		);

		//this->m_temp_allocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		//this->m_job_system = new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1);


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
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(dimensions);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);

		JPH::BoxShapeSettings box_shape_settings(dim);
		JPH::ShapeSettings::ShapeResult box_shape_result = box_shape_settings.Create();
		if (box_shape_result.HasError())
		{
			s_JoltData->LastErrorMessage = box_shape_result.GetError();
		}
		assert(!box_shape_result.HasError());	// TODO: remove?
		JPH::ShapeRefC box_shape = box_shape_result.Get(); // We don't expect an error here, but you can check floor_shape_result for HasError() / GetError()


		JPH::BodyCreationSettings* box_settings;
		if (movability == HEMotionType::Static)
		{
			box_settings = new JPH::BodyCreationSettings(new JPH::BoxShape(dim), pos, JPH::Quat::sIdentity(), mov, Layers::NON_MOVING);
		}
		else
		{
			box_settings = new JPH::BodyCreationSettings(box_shape, pos, JPH::Quat::sIdentity(), mov, Layers::MOVING);
			// Note: it is possible to have more than two layers but that has not been implemented yet
		}

		JPH::BodyID box_id;
		// Add it to the world
		if (activate)
		{
			box_id = (this->m_body_interface)->CreateAndAddBody(*box_settings, JPH::EActivation::Activate);
		}
		else
		{
			box_id = (this->m_body_interface)->CreateAndAddBody(*box_settings, JPH::EActivation::DontActivate);
		}

		return PhysicsEngine::makeHBodyID(box_id);;
	}

	HBodyID PhysicsEngine::createSphere(float radius, HVec3 position, HEMotionType movability, bool activate)
	{
		//conversion
		JPH::RVec3 pos = PhysicsEngine::makeRVec3(position);
		JPH::EMotionType mov = PhysicsEngine::makeEMotionType(movability);

		JPH::SphereShapeSettings sphere_shape_settings(radius);		// TODO: I think we can add material here
		JPH::ShapeSettings::ShapeResult sphere_shape_result = sphere_shape_settings.Create();
		if (sphere_shape_result.HasError())
		{
			s_JoltData->LastErrorMessage = sphere_shape_result.GetError();
		}
		JPH::ShapeRefC sphere_shape = sphere_shape_result.Get();
		JPH::BodyCreationSettings* sphere_settings;
		if (mov == JPH::EMotionType::Static)
		{
			sphere_settings = new JPH::BodyCreationSettings(sphere_shape, pos, JPH::Quat::sIdentity(), mov, Layers::NON_MOVING);
		}
		else
		{
			sphere_settings = new JPH::BodyCreationSettings(sphere_shape, pos, JPH::Quat::sIdentity(), mov, Layers::NON_MOVING);
		}
		JPH::BodyID sphere_id;
		if (activate)
		{
			sphere_id = (this->m_body_interface)->CreateAndAddBody(*sphere_settings, JPH::EActivation::Activate);
		}
		else
		{
			sphere_id = (this->m_body_interface)->CreateAndAddBody(*sphere_settings, JPH::EActivation::DontActivate);
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

		JPH::Vec3 vec = (this->m_body_interface)->GetCenterOfMassPosition(jolt_id);
		return PhysicsEngine::makeHVec3(vec);
	}

	// TODO: delete tmp function below
	void PhysicsEngine::tmpRunner()
	{
		/*	TODO: Rework this
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
		*/

	}
}
