#pragma once

namespace Engine {

	class UUID64
	{
	public:
		UUID64();
		UUID64(uint64_t uuid);
		UUID64(const UUID64& other);

		operator uint64_t () { return m_UUID; }
		operator const uint64_t() const { return m_UUID; }
	private:
		uint64_t m_UUID;
	};

	class UUID
	{
	public:
		UUID();
		UUID(uint32_t uuid);
		UUID(const UUID& other);

		operator uint32_t () { return m_UUID; }
		operator const uint32_t() const { return m_UUID; }
	private:
		uint32_t m_UUID;
	};

}

namespace std {

	template <>
	struct hash<Engine::UUID64>
	{
		std::size_t operator()(const Engine::UUID64& uuid) const
		{
			// uuid is already a randomly generated number, and is suitable as a hash key as-is.
			// this may change in future, in which case return hash<uint64_t>{}(uuid); might be more appropriate
			return uuid;
		}
	};

	template <>
	struct hash<Engine::UUID>
	{
		std::size_t operator()(const Engine::UUID& uuid) const
		{
			return hash<uint32_t>()((uint32_t)uuid);
		}
	};
}