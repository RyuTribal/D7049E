#include "pch.h"

#include "HContactListener.h"

namespace Engine {

	JPH::ValidateResult	HContactListener::OnContactValidate(const JPH::Body& inBody1, const JPH::Body& inBody2, JPH::RVec3Arg inBaseOffset, const JPH::CollideShapeResult& inCollisionResult)
	{
		if (typeid(inBody1.GetShape()) == typeid(JPH::CapsuleShape))
		{
			HVE_CORE_TRACE("Contact validate callback");

		}

		// Allows you to ignore a contact before it is created (using layers to not make objects collide is cheaper!)
		return JPH::ValidateResult::AcceptAllContactsForThisBodyPair;
	}

	void HContactListener::OnContactAdded(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		std::uint64_t data1 = (std::uint64_t)inBody1.GetUserData();
		std::uint64_t data2 = (std::uint64_t)inBody2.GetUserData();

		this->m_CurrentScene->AddNewContact(data1, data2);

		if (typeid(inBody1.GetShape()) == typeid(JPH::CapsuleShape))
			HVE_CORE_TRACE("A contact was added");
	}

	void HContactListener::OnContactPersisted(const JPH::Body& inBody1, const JPH::Body& inBody2, const JPH::ContactManifold& inManifold, JPH::ContactSettings& ioSettings)
	{
		// Todo later
	}

	void HContactListener::OnContactRemoved(const JPH::SubShapeIDPair& inSubShapePair)
	{
		JPH::BodyID id1 = inSubShapePair.GetBody1ID();
		JPH::BodyID id2 = inSubShapePair.GetBody2ID();

		JPH::Body* body1 = m_BodyLockInterface->TryGetBody(id1);
		JPH::Body* body2 = m_BodyLockInterface->TryGetBody(id2);

		if (body1 == nullptr || body2 == nullptr)
		{
			return;
		}

		UUID data1 = body1->GetUserData();
		UUID data2 = body2->GetUserData();

		this->m_CurrentScene->AddRemoveContact(data1, data2);
		HVE_CORE_TRACE("A contact was removed");
	}

	void HContactListener::SetCurrentScene(HPhysicsScene* current_scene)
	{
		this->m_CurrentScene = current_scene;
	}
}

