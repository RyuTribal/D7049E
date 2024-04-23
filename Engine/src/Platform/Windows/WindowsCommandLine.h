#pragma once

#include "Core/IO.h";

namespace Engine {
	class WindowsCommandLine : public CommandLine
	{
	public:
		void ExecuteCommand(std::string& command, CommandArgs& arguments) override;
		std::string GetLastErrorAsString();
	};
}
