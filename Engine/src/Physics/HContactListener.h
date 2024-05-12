#pragma once

#ifndef HContactListener_h
#define HContactListener_h

#include "HPhysicsScene.h"

namespace Engine {

	class HContactListener : public JPH::ContactListener
	{
	public:

		HContactListener(HPhysicsScene* physicsScene) : m_CurrentScene(physicsScene) {
			m_BodyLockInterface = (JPH::BodyLockInterfaceNoLock*)&physicsScene->GetSystem()->GetBodyInterfaceNoLock();
		}

		virtual JPH::ValidateResult	OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult) override;

		virtual void OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings) override;

		virtual void OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair) override;

		void SetCurrentScene(HPhysicsScene* current_scene);

	private:
		const JPH::BodyLockInterfaceNoLock* m_BodyLockInterface = nullptr;
		HPhysicsScene* m_CurrentScene;
	};

}

#endif // !MyContactListener_h
