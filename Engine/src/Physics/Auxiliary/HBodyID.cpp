#include "pch.h"
#include "HBodyID.h"

namespace Engine {
	HBodyID::HBodyID() :
			mID(cInvalidBodyID)
	{
	}

	/// Construct from index and sequence number combined in a single uint32_t (use with care!)
	explicit HBodyID::HBodyID(std::uint32_t inID) : mID(inID)
	{
		//JPH_ASSERT((inID & cBroadPhaseBit) == 0 || inID == cInvalidBodyID); // Check bit used by broadphase
		HVE_ASSERT((inID & cBroadPhaseBit) == 0 || inID == cInvalidBodyID);
	}

	/// Construct from index and sequence number
	explicit HBodyID::HBodyID(std::uint32_t inID, std::uint8_t inSequenceNumber) : mID((uint32_t(inSequenceNumber) << 24) | inID)
	{
		//JPH_ASSERT(inID < cMaxBodyIndex); // Should not use bit pattern for invalid ID and should not use the broadphase bit
		HVE_ASSERT(inID < cMaxBodyIndex);
	}

	/// Get index in body array
	inline std::uint32_t HBodyID::GetIndex() const
	{
		return mID & cMaxBodyIndex;
	}

	/// Get sequence number of body.
	/// The sequence number can be used to check if a body ID with the same body index has been reused by another body.
	/// It is mainly used in multi threaded situations where a body is removed and its body index is immediately reused by a body created from another thread.
	/// Functions querying the broadphase can (after aquiring a body lock) detect that the body has been removed (we assume that this won't happen more than 128 times in a row).
	inline std::uint8_t	HBodyID::GetSequenceNumber() const
	{
		return uint8_t(mID >> 24);
	}

	/// Returns the index and sequence number combined in an uint32_t
	inline std::uint32_t			HBodyID::GetIndexAndSequenceNumber() const
	{
		return mID;
	}

	/// Check if the ID is valid
	inline bool HBodyID::IsInvalid() const
	{
		return mID == cInvalidBodyID;
	}

	/// Equals check
	inline bool	HBodyID::operator == (const HBodyID& inRHS) const
	{
		return mID == inRHS.mID;
	}

	/// Not equals check
	inline bool HBodyID::operator != (const HBodyID& inRHS) const
	{
		return mID != inRHS.mID;
	}

	/// Smaller than operator, can be used for sorting bodies
	inline bool HBodyID::operator < (const HBodyID& inRHS) const
	{
		return mID < inRHS.mID;
	}

	/// Greater than operator, can be used for sorting bodies
	inline bool HBodyID::operator > (const HBodyID& inRHS) const
	{
		return mID > inRHS.mID;
	}
}
