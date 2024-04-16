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

	void PhysicsEngine::Init()
	{
		JPH::RegisterDefaultAllocator();

		JPH::Trace = JoltTraceCallback;

		JPH::Factory::sInstance = new JPH::Factory();

		JPH::RegisterTypes();

		s_JoltData = new JoltData();

	}

	void PhysicsEngine::Shutdown()
	{
		delete s_JoltData->TemporariesAllocator;
		delete s_JoltData;
		delete JPH::Factory::sInstance;
	}
}
