#include "pch.h"
#include "AssetRegistry.h"

namespace Engine {
	static std::mutex s_AssetRegistryMutex; // In case we want to read/write from different threads. I am planning on moving the renderer to a seperate thread, so this will probably be useful

	AssetMetadata& AssetRegistry::operator[](const AssetHandle handle)
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		HVE_CORE_TRACE_TAG("Asset Registry", "Retrieving handle {}", handle);
		return m_AssetRegistry[handle];
	}

	const AssetMetadata& AssetRegistry::Get(const AssetHandle handle) const
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		HVE_CORE_ASSERT(m_AssetRegistry.find(handle) != m_AssetRegistry.end());
		//HVE_CORE_TRACE_TAG("Asset Registry", "Retrieving const handle {}", handle);
		return m_AssetRegistry.at(handle);
	}

	AssetMetadata& AssetRegistry::Get(const AssetHandle handle)
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		//HVE_CORE_TRACE_TAG("Asset Registry", "Retrieving handle {}", handle);
		return m_AssetRegistry[handle];
	}

	bool AssetRegistry::Contains(const AssetHandle handle) const
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		//HVE_CORE_TRACE_TAG("Asset Registry", "Contains handle {}", handle);
		return m_AssetRegistry.find(handle) != m_AssetRegistry.end();
	}

	size_t AssetRegistry::Remove(const AssetHandle handle)
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		HVE_CORE_TRACE_TAG("Asset Registry", "Removing handle", handle);
		return m_AssetRegistry.erase(handle);
	}

	void AssetRegistry::Clear()
	{
		std::scoped_lock<std::mutex> lock(s_AssetRegistryMutex);

		HVE_CORE_TRACE_TAG("Asset Registry", "Clearing registry");
		m_AssetRegistry.clear();
	}


}
