#include "pch.h"
#include "UUID.h"

#include <random>

#include <unordered_map>

namespace Engine {

	static std::random_device s_RandomDevice;
	static std::mt19937_64 eng(s_RandomDevice());
	static std::uniform_int_distribution<uint64_t> s_UniformDistribution;

	static std::mt19937 eng32(s_RandomDevice());
	static std::uniform_int_distribution<uint32_t> s_UniformDistribution32;

	UUID64::UUID64()
		: m_UUID(s_UniformDistribution(eng))
	{
	}

	UUID64::UUID64(uint64_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID64::UUID64(const UUID64& other)
		: m_UUID(other.m_UUID)
	{
	}


	UUID::UUID()
		: m_UUID(s_UniformDistribution32(eng32))
	{
	}

	UUID::UUID(uint32_t uuid)
		: m_UUID(uuid)
	{
	}

	UUID::UUID(const UUID& other)
		: m_UUID(other.m_UUID)
	{
	}

}