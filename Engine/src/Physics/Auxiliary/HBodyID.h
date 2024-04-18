#pragma once
//#include <Jolt/Core/HashCombine.h>	TODO: if complain: add this maybe

#include <stdint.h>


namespace Engine {
	class HBodyID
	{

		public:

			static constexpr std::uint32_t	cInvalidBodyID = 0xffffffff;	///< The value for an invalid body ID
			static constexpr std::uint32_t	cBroadPhaseBit = 0x00800000;	///< This bit is used by the broadphase
			static constexpr std::uint32_t	cMaxBodyIndex = 0x7fffff;		///< Maximum value for body index (also the maximum amount of bodies supported - 1)
			static constexpr std::uint8_t	cMaxSequenceNumber = 0xff;		///< Maximum value for the sequence number

			HBodyID();

			HBodyID(std::uint32_t inID);

			explicit HBodyID(std::uint32_t inID, std::uint8_t inSequenceNumber);

			inline std::uint32_t GetIndex() const;

			inline std::uint8_t GetSequenceNumber() const;

			inline std::uint32_t GetIndexAndSequenceNumber() const;

			inline bool IsInvalid() const;

			inline bool operator == (const HBodyID& inRHS) const;

			inline bool operator != (const HBodyID& inRHS) const;

			inline bool operator < (const HBodyID& inRHS) const;

			inline bool operator > (const HBodyID& inRHS) const;

		private:
			uint32_t mID;

			// Create a std::hash for BodyID
			//JPH_MAKE_HASHABLE(JPH::BodyID, t.GetIndexAndSequenceNumber())	// TODO: this??

	};

}

