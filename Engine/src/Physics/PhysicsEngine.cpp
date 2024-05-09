#include "pch.h"
#include "PhysicsEngine.h"

#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>


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

	

	PhysicsEngine* PhysicsEngine::s_Instance = nullptr;
	PhysicsScene* PhysicsEngine::s_current_scene = nullptr;

	void PhysicsEngine::Init(int allocationSize)
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JoltTraceCallback;

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_JoltData = new JoltData();

		s_JoltData->TemporariesAllocator = new JPH::TempAllocatorImpl(allocationSize * 1024 * 1024);
		s_JoltData->JobThreadPool = std::unique_ptr<JPH::JobSystemThreadPool>(
			new JPH::JobSystemThreadPool(JPH::cMaxPhysicsJobs, JPH::cMaxPhysicsBarriers, JPH::thread::hardware_concurrency() - 1)
		);
	}

	void PhysicsEngine::Shutdown()
	{
		for (auto& [scene_id, scene] : s_sceneMap)
		{
			delete scene;
		}

		delete s_JoltData->TemporariesAllocator;
		delete s_JoltData;

		JPH::UnregisterTypes();

		// Destroy the factory
		delete JPH::Factory::sInstance;
		JPH::Factory::sInstance = nullptr;

		s_current_scene = nullptr;
		s_sceneMap.clear();
	}



	void PhysicsEngine::Step(float deltaTime)
	{

		s_current_scene->Update(deltaTime);

		// TODO: Add post simulation step 
	}

	

	void PhysicsEngine::OnRuntimeStart(int collisionSteps, int integrationSubStep)
	{
		s_current_scene->SetCollisionAndIntegrationSteps(collisionSteps, integrationSubStep);

	}

	void PhysicsEngine::OnRuntimeStop()
	{
		s_current_scene->SetOptimized(false);
		s_current_scene->DestroyAll();
	}

	SceneID PhysicsEngine::CreateScene(Scene* scene, float allocationSize = 10.0f)
	{
		PhysicsScene* phys_scene = new PhysicsScene(scene, s_JoltData->TemporariesAllocator, s_JoltData->JobThreadPool.get());
		SceneID id = SceneID();
		s_sceneMap[id] = phys_scene;
		if (s_current_scene == nullptr)
		{
			s_current_scene = phys_scene;
		}

		return id;
	}

	PhysicsScene* PhysicsEngine::GetScene(SceneID sceneID)
	{
		return s_sceneMap[sceneID];
	}

	PhysicsScene* PhysicsEngine::GetCurrentScene()
	{
		return this->s_current_scene;
	}

	// TODO: add a way to handle scene directly

	void PhysicsEngine::SwitchScene(SceneID sceneID)
	{
		if (s_sceneMap.contains(sceneID))
		{
			s_current_scene = s_sceneMap[sceneID];
		}
		else
		{
			HVE_CONSOLE_LOG_ERROR("No scene with that sceneID");
			HVE_ASSERT(false);
		}
	}

	void PhysicsEngine::tmpRunner()
	{
		PhysicsEngine* engin = PhysicsEngine::Get();

		engin->Init(10);
		Scene scn = Scene("Boogey");
		SceneID scene_id = engin->CreateScene(&scn);
		PhysicsScene* scene = engin->GetScene(scene_id);

		UUID box = UUID();
		UUID sphere = UUID();

		glm::vec3 offset = glm::vec3(0.0f, 0.0f, 0.0f);
		HBodyID box_id = scene->CreateBox(box, glm::vec3(100.0f, 1.0f, 100.0f), glm::vec3(0.0, -1.0, 0.0), HEMotionType::Static, offset, false);
		HBodyID sphere_id = scene->CreateSphere(sphere, 0.5f, glm::vec3(0.0, 2.0, 0.0), HEMotionType::Dynamic, offset, true);
		//HBodyID sphere_id = engin->CreateBox(glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0, 2.0, 0.0), HEMotionType::Dynamic, true);
		glm::vec3 velocity = glm::vec3(0.0f, -5.0f, 0.0f);
		scene->SetLinearVelocity(sphere, velocity);
		scene->OptimizeBroadPhase();

		int stepCounter = 0;
		std::cout << "Starting simulation" << std::endl;
		while (scene->IsActive(sphere_id) && stepCounter < 200)
		{
			++stepCounter;
			glm::vec3 position = scene->GetCenterOfMassPosition(sphere);
			glm::vec3 velocity = scene->GetLinearVelocity(sphere);
			std::cout << "Step " << stepCounter << ": Position = (" << position.x << ", " << position.y << ", " << position.z << "), Velocity = (" << velocity.x << ", " << velocity.y << ", " << velocity.z << ")" << std::endl;
			engin->Step(1.0 / 60.0);

		}
		std::cout << "Finished the tmp simulation" << std::endl;

		engin->Shutdown();
	}
}
