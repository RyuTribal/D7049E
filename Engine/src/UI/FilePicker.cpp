#include "FilePicker.h"
#include <nfd.h>
#include <Engine.h>

namespace Engine {
	std::pair<bool, std::string> FilePicker::OpenFileExplorer(std::vector<std::vector<std::string>>& filter, bool pick_folder = false)
	{
		std::string path_result = "";
		NFD_Init();

		bool selected = false;

		nfdchar_t* outPath;
		const int filter_size = (int)filter.size();
		nfdfilteritem_t* filterItem = new nfdfilteritem_t[filter_size];
		for (size_t i = 0; i < filter.size(); i++)
		{
			filterItem[i] = { filter[i][0].c_str(), filter[i][1].c_str() };
		}

		nfdresult_t result;
		if (pick_folder)
		{
			result = NFD_PickFolder(&outPath, NULL);
		}
		else
		{
			result = NFD_OpenDialog(&outPath, filterItem, filter_size, NULL);
		}

		if (result == NFD_OKAY)
		{
			path_result = outPath;
			NFD_FreePath(outPath);
			selected = true;
		}
		else if (result == NFD_CANCEL)
		{
			
		}
		else
		{
			HVE_CORE_ERROR_TAG("File Picker", "Error: {0}", NFD_GetError());
		}


		NFD_Quit();
		delete[] filterItem;
		return { selected, path_result };
	}

	std::pair<bool, std::string> FilePicker::OpenFileExplorer(bool pick_folder = false)
	{
		std::vector<std::vector<std::string>> filter{};
		return OpenFileExplorer(filter, pick_folder);
	}

}
