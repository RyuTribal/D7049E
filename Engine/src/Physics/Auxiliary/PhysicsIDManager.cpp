#include "pch.h"
#include "PhysicsIDManager.h"

namespace Engine {

	HBodyID::HBodyID(UUID entity_id, JPH::BodyID value)
	{
		mID = entity_id;
		s_idMap[entity_id] = value;
	}

	/*UUID HBodyID::InsertNewID(JPH::BodyID value)
	{
		if (sID == UINT32_MAX)
		{
			HVE_CONSOLE_LOG_ERROR("The BodyID counter has gotten too large");
		}
		s_idMap[sID] = value;
		return sID++;
	}*/

	JPH::BodyID HBodyID::GetBodyID(UUID id)
	{
		return s_idMap[id];
	}

	JPH::BodyID HBodyID::GetBodyID()
	{
		return s_idMap[this->mID];
	}

	void HBodyID::EmptyMap()
	{
		s_idMap.clear();
	}

	void HBodyID::RemoveEntry(UUID entity_id)
	{
		s_idMap.erase(entity_id);
	}

	std::unordered_map<UUID, JPH::BodyID>& HBodyID::GetMap()
	{
		return s_idMap;
	}

	
	/*
	/// Get index in body array
	std::uint32_t HBodyID::GetIndex() const
	{
		return mID & cMaxBodyIndex;
	}

	/// Get sequence number of body.
	/// The sequence number can be used to check if a body ID with the same body index has been reused by another body.
	/// It is mainly used in multi threaded situations where a body is removed and its body index is immediately reused by a body created from another thread.
	/// Functions querying the broadphase can (after aquiring a body lock) detect that the body has been removed (we assume that this won't happen more than 128 times in a row).
	std::uint8_t	HBodyID::GetSequenceNumber() const
	{
		return uint8_t(mID >> 24);
	}

	/// Returns the index and sequence number combined in an uint32_t
	 std::uint32_t			HBodyID::GetIndexAndSequenceNumber() const
	{
		return mID;
	}
	*/

	SceneID::SceneID() {
		this->m_id = s_id++;
	}

}
