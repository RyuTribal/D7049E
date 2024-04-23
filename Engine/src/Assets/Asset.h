#pragma once
#include "AssetTypes.h"


namespace Engine {

	using AssetHandle = UUID; // This is so there are no conflicting types if you choose to create an asset but also have your own UUID for id management and you happen to call it a m_Handle

	class Asset
	{
	public:
		virtual AssetType GetType() const = 0;
		AssetHandle Handle; // Generates automatically on creation. Check UUID class for info

	};
}
