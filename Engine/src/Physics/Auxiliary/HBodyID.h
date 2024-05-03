#pragma once
//#include <Jolt/Core/HashCombine.h>	TODO: if complain: add this maybe

#include <stdint.h>

#include <Jolt/Physics/Body/BodyCreationSettings.h>



namespace Engine {
	class HBodyID
	{
		public:
			HBodyID(UUID entity_id, JPH::BodyID value);
			// static UUID InsertNewID(JPH::BodyID value);
			static JPH::BodyID GetBodyID(UUID id);		// TODO: remove these static functions
			JPH::BodyID GetBodyID();

			static void EmptyMap();

			static std::unordered_map<UUID, JPH::BodyID>& GetMap();


		private:
			UUID mID;

			static inline std::unordered_map<UUID, JPH::BodyID> s_idMap = std::unordered_map<UUID, JPH::BodyID>();
			// Create a std::hash for BodyID
			//JPH_MAKE_HASHABLE(JPH::BodyID, t.GetIndexAndSequenceNumber())	// TODO: this??

	};

}

