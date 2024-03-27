#pragma once

#include "Core.h"
// This ignores all warnings raised inside External headers
#pragma warning(push, 0)
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"
#pragma warning(pop)


namespace Engine
{
	class Log
	{
	public:
		static void Init();

		inline static Ref<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
	private:
		static Ref<spdlog::logger> s_CoreLogger;
	};
}

// Core log macros
#define CORE_FATAL(...) ::Engine::Log::GetCoreLogger()->critical(__VA_ARGS__)
#define CORE_ERROR(...) ::Engine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define CORE_WARN(...) ::Engine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define CORE_INFO(...) ::Engine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define CORE_TRACE(...) :: Engine::Log::GetCoreLogger()->trace(__VA_ARGS__)
