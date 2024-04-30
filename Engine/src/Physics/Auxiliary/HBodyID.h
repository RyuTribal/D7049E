#pragma once
//#include <Jolt/Core/HashCombine.h>	TODO: if complain: add this maybe

#include <stdint.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>



namespace Engine {
	class HBodyID
	{
		public:
			HBodyID(JPH::BodyID value);
			static uint32_t InsertNewID(JPH::BodyID value);
			static JPH::BodyID GetBodyID(uint32_t id);		// TODO: remove these static functions
			JPH::BodyID GetBodyID();


		private:
			uint32_t mID;
			static inline uint32_t sID = 0;

			static inline std::unordered_map<std::uint32_t, JPH::BodyID> s_idMap = std::unordered_map<std::uint32_t, JPH::BodyID>();
			// Create a std::hash for BodyID
			//JPH_MAKE_HASHABLE(JPH::BodyID, t.GetIndexAndSequenceNumber())	// TODO: this??

	};

}

