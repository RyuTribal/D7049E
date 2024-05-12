#pragma once
//#include <Jolt/Core/TempAllocator.h>
//#include <Jolt/Core/JobSystemThreadPool.h>
//#include <Jolt/RegisterTypes.h>
//#include <Jolt/Core/Factory.h>
//#include <Jolt/Physics/PhysicsSettings.h>
//#include <Jolt/Physics/PhysicsSystem.h>
//#include <Jolt/Physics/Body/BodyCreationSettings.h>
//#include <Jolt/Physics/Body/BodyActivationListener.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
//#include <Jolt/Physics/Collision/Shape/SphereShape.h>
//#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
//#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
////#include <Jolt/Physics/Character/CharacterVirtual.h>
//#include <Jolt/Physics/Character/Character.h>
//
//#include "HVec3.h"
//#include "HVec3.h"
//#include "HEMotionType.h"
//#include "PhysicsIDManager.h"
//
//#include "Scene/Entity.h"
//#include "Scene/Components.h"

// TODO fix the imports




namespace Engine {

	namespace BroadPhaseLayers {
		static constexpr JPH::BroadPhaseLayer NON_MOVING(0);
		static constexpr JPH::BroadPhaseLayer MOVING(1);
		static constexpr JPH::uint NUM_LAYERS(2);
	};

	namespace Layers {
		static constexpr JPH::ObjectLayer NON_MOVING = 0;
		static constexpr JPH::ObjectLayer MOVING = 1;
		static constexpr JPH::ObjectLayer NUM_LAYERS = 2;
	};
	
}

