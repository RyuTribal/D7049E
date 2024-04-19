#pragma once


namespace Engine {

	struct CommandArgs
	{
		bool NewProcess = false;
		bool UseAnotherWorkingDir = false;
		bool SleepUntilFinished = false;
		std::string WorkingDir = "";
	};

	class CommandLine
	{
	public:
		virtual ~CommandLine() {};

		static Ref<CommandLine> Create();
		virtual void ExecuteCommand(std::string& command, CommandArgs& arguments) = 0;

	private:
	};
}
