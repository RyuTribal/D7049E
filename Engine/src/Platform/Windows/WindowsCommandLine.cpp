#include "pch.h"
#include "WindowsCommandLine.h"


namespace Engine {

	std::string WindowsCommandLine::GetLastErrorAsString()
	{
		DWORD errorMessageID = ::GetLastError();
		if (errorMessageID == 0)
		{
			return std::string();
		}

		LPSTR messageBuffer = nullptr;
		size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
									 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);

		std::string message(messageBuffer, size);
		LocalFree(messageBuffer);

		return message;
	}

	//NULL,   // No module name (use command line)
	//	& command[0],            // Command line (mutable string required)
	//	NULL,                   // Process handle not inheritable
	//	NULL,                   // Thread handle not inheritable
	//	FALSE,                  // Set handle inheritance to FALSE
	//	0,                      // No creation flags
	//	NULL,                   // Use parent's environment block
	//	NULL,                   // Use parent's starting directory 
	//	& si,                    // Pointer to STARTUPINFO structure
	//	& pi

	void WindowsCommandLine::ExecuteCommand(std::string& command, CommandArgs& arguments)
	{
		HVE_CORE_TRACE_TAG("CommandLine", "Attempting to run command: {}", command);

		STARTUPINFO si{};
		PROCESS_INFORMATION pi{};
		std::wstring commandW(command.begin(), command.end());
		std::wstring workingDirW(arguments.WorkingDir.begin(), arguments.WorkingDir.end());

		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = SW_HIDE;

		std::vector<wchar_t> commandVec(commandW.begin(), commandW.end());
		commandVec.push_back(0);
		if (!CreateProcessW(NULL, commandVec.data(), NULL, NULL, FALSE, arguments.NewProcess ? CREATE_NEW_CONSOLE : 0, NULL, arguments.UseAnotherWorkingDir ? workingDirW.c_str() : NULL, &si, &pi))
		{
			HVE_CORE_ERROR_TAG("CommandLine", "CreateProcess failed with error: {}", GetLastErrorAsString());
			return;
		}

		if (arguments.SleepUntilFinished)
		{
			WaitForSingleObject(pi.hProcess, INFINITE);

			DWORD exitCode;
			GetExitCodeProcess(pi.hProcess, &exitCode);

			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);

			if (exitCode != 0)
			{
				HVE_CORE_ERROR_TAG("CommandLine", "CreateProcess failed with exit code {}", exitCode);
				return;
			}

			HVE_CORE_TRACE_TAG("CommandLine", "Command executed successfully with exit code {}", exitCode);
		}
		
	}

	Ref<CommandLine> CommandLine::Create()
	{
		return CreateRef<WindowsCommandLine>();
	}
}
