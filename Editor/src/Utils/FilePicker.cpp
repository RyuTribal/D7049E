#include "FilePicker.h"
#include <nfd.h>
#include <Engine.h>

namespace Editor {
	std::string FilePicker::OpenFileExplorer(std::vector<std::vector<std::string>>& filter)
	{
		std::string path_result = "";
		NFD_Init();

		nfdchar_t* outPath;
		const int filter_size = (int)filter.size();
		nfdfilteritem_t* filterItem = new nfdfilteritem_t[filter_size];
		for (size_t i = 0; i < filter.size(); i++)
		{
			filterItem[i] = { filter[i][0].c_str(), filter[i][1].c_str() };
		}

		nfdresult_t result = NFD_OpenDialog(&outPath, filterItem, filter_size, NULL);

		if (result == NFD_OKAY)
		{
			path_result = outPath;
			NFD_FreePath(outPath);
		}
		else if (result == NFD_CANCEL)
		{
			
		}
		else
		{
			HVE_ERROR_TAG("File Picker", "Error: {0}", NFD_GetError());
		}


		NFD_Quit();
		delete[] filterItem;
		return path_result;
	}

}
