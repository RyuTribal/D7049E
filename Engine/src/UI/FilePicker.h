#pragma once
#include <string>
#include <vector>


namespace Engine {
	class FilePicker
	{
	public:
		static std::pair<bool, std::string> OpenFileExplorer(std::vector<std::vector<std::string>>& filter, bool pick_folder);
		static std::pair<bool, std::string> OpenFileExplorer(bool pick_folder);
	private:

	};
}
