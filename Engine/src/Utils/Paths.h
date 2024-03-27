#pragma once


namespace Engine 
{
	class Paths 
	{
	public:
		static std::string Get(const std::string& key) {
			try {
				return paths.at(key);
			}
			catch (const std::out_of_range&) {
				THROW_CORE_ERROR("The path does not exist!");
			}
		}

	private:
		static const std::unordered_map<std::string, std::string> paths;
	};
}