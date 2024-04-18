#pragma once
#include <string>
#include <vector>


namespace Engine {
	class FilePicker
	{
	public:
		static std::string OpenFileExplorer(std::vector<std::vector<std::string>>& filter, bool pick_folder);
		static std::string OpenFileExplorer(bool pick_folder);
	private:

	};
}
