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
			static JPH::BodyID GetBodyID(UUID id);
			JPH::BodyID GetBodyID();

			static void EmptyMap();

			static void RemoveEntry(UUID entity_id);

			static std::unordered_map<UUID, JPH::BodyID>& GetMap();


		private:
			UUID mID;

			static inline std::unordered_map<UUID, JPH::BodyID> s_idMap = std::unordered_map<UUID, JPH::BodyID>();

	};

	class SceneID
	{
	public:
		SceneID();


		inline bool operator < (const SceneID& inScene) const
		{
			return m_id < inScene.m_id;
		}

		inline bool operator > (const SceneID& inScene) const
		{
			return m_id > inScene.m_id;
		}

	private:
		inline static unsigned int s_id = 0;

		unsigned int m_id;
	};

}

