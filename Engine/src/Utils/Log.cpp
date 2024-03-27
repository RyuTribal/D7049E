#include "pch.h"
#include <spdlog/sinks/stdout_color_sinks.h>

namespace Engine
{
	Ref<spdlog::logger> Log::s_CoreLogger;
	void Log::Init()
	{
		spdlog::set_pattern("%^[%T] %n: %v%$");
		s_CoreLogger = spdlog::stdout_color_mt("Program");
		s_CoreLogger->set_level(spdlog::level::trace);
	}



}
