#pragma once

#include "Core/Base.h"
#include "LogCustomFormatters.h"

#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

#include <map>

#define HVE_ASSERT_MESSAGE_BOX (!DIST && PLATFORM_WINDOWS)

#if HVE_ASSERT_MESSAGE_BOX
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif
#endif

namespace Engine {

	class Log
	{
	public:
		enum class Type : uint8_t
		{
			Core = 0, Client = 1
		};
		enum class Level : uint8_t
		{
			Trace = 0, Info, Warn, Error, Fatal
		};
		struct TagDetails
		{
			bool Enabled = true;
			Level LevelFilter = Level::Trace;
		};

	public:
		static void Init();
		static void Shutdown();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetEditorConsoleLogger() { return s_EditorConsoleLogger; }

		static bool HasTag(const std::string& tag) { return s_EnabledTags.find(tag) != s_EnabledTags.end(); }
		static std::map<std::string, TagDetails>& EnabledTags() { return s_EnabledTags; }

		template<typename... Args>
		static void PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args);

		template<typename... Args>
		static void PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args);

	public:
		// Enum utils
		static const char* LevelToString(Level level)
		{
			switch (level)
			{
			case Level::Trace: return "Trace";
			case Level::Info:  return "Info";
			case Level::Warn:  return "Warn";
			case Level::Error: return "Error";
			case Level::Fatal: return "Fatal";
			}
			return "";
		}
		static Level LevelFromString(std::string_view string)
		{
			if (string == "Trace") return Level::Trace;
			if (string == "Info")  return Level::Info;
			if (string == "Warn")  return Level::Warn;
			if (string == "Error") return Level::Error;
			if (string == "Fatal") return Level::Fatal;

			return Level::Trace;
		}

	private:
		static std::shared_ptr<spdlog::logger> s_CoreLogger;
		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_EditorConsoleLogger;

		inline static std::map<std::string, TagDetails> s_EnabledTags;
	};

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Tagged logs (prefer these!)                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core logging
#define HVE_CORE_TRACE_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Trace, tag, __VA_ARGS__)
#define HVE_CORE_INFO_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Info, tag, __VA_ARGS__)
#define HVE_CORE_WARN_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Warn, tag, __VA_ARGS__)
#define HVE_CORE_ERROR_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Error, tag, __VA_ARGS__)
#define HVE_CORE_FATAL_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Fatal, tag, __VA_ARGS__)

// Client logging
#define HVE_TRACE_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Trace, tag, __VA_ARGS__)
#define HVE_INFO_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Info, tag, __VA_ARGS__)
#define HVE_WARN_TAG(tag, ...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Warn, tag, __VA_ARGS__)
#define HVE_ERROR_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Error, tag, __VA_ARGS__)
#define HVE_FATAL_TAG(tag, ...) ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Fatal, tag, __VA_ARGS__)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Core Logging
#define HVE_CORE_TRACE(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Trace, "", __VA_ARGS__)
#define HVE_CORE_INFO(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Info, "", __VA_ARGS__)
#define HVE_CORE_WARN(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Warn, "", __VA_ARGS__)
#define HVE_CORE_ERROR(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Error, "", __VA_ARGS__)
#define HVE_CORE_FATAL(...)  ::Engine::Log::PrintMessage(::Engine::Log::Type::Core, ::Engine::Log::Level::Fatal, "", __VA_ARGS__)

// Client Logging
#define HVE_TRACE(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Trace, "", __VA_ARGS__)
#define HVE_INFO(...)    ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Info, "", __VA_ARGS__)
#define HVE_WARN(...)    ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Warn, "", __VA_ARGS__)
#define HVE_ERROR(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Error, "", __VA_ARGS__)
#define HVE_FATAL(...)   ::Engine::Log::PrintMessage(::Engine::Log::Type::Client, ::Engine::Log::Level::Fatal, "", __VA_ARGS__)

// Editor Console Logging Macros
#define HVE_CONSOLE_LOG_TRACE(...)   Engine::Log::GetEditorConsoleLogger()->trace(__VA_ARGS__)
#define HVE_CONSOLE_LOG_INFO(...)    Engine::Log::GetEditorConsoleLogger()->info(__VA_ARGS__)
#define HVE_CONSOLE_LOG_WARN(...)    Engine::Log::GetEditorConsoleLogger()->warn(__VA_ARGS__)
#define HVE_CONSOLE_LOG_ERROR(...)   Engine::Log::GetEditorConsoleLogger()->error(__VA_ARGS__)
#define HVE_CONSOLE_LOG_FATAL(...)   Engine::Log::GetEditorConsoleLogger()->critical(__VA_ARGS__)

namespace Engine {

	template<typename... Args>
	void Log::PrintMessage(Log::Type type, Log::Level level, std::string_view tag, Args&&... args)
	{
		auto detail = s_EnabledTags[std::string(tag)];
		if (detail.Enabled && detail.LevelFilter <= level)
		{
			auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
			std::string logString = tag.empty() ? "{0}{1}" : "[{0}] {1}";
			switch (level)
			{
			case Level::Trace:
				logger->trace(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
				case Level::Info:
				logger->info(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Warn:
				logger->warn(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Error:
				logger->error(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			case Level::Fatal:
				logger->critical(logString, tag, fmt::format(std::forward<Args>(args)...));
				break;
			}
		}
	}


	template<typename... Args>
	void Log::PrintAssertMessage(Log::Type type, std::string_view prefix, Args&&... args)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}: {1}", prefix, fmt::format(std::forward<Args>(args)...));

#if HVE_ASSERT_MESSAGE_BOX
		std::string message = fmt::format(std::forward<Args>(args)...);
		MessageBoxA(nullptr, message.c_str(), "Helios Assert", MB_OK | MB_ICONERROR);
#endif
	}

	template<>
	inline void Log::PrintAssertMessage(Log::Type type, std::string_view prefix)
	{
		auto logger = (type == Type::Core) ? GetCoreLogger() : GetClientLogger();
		logger->error("{0}", prefix);
#if HVE_ASSERT_MESSAGE_BOX
		MessageBoxA(nullptr, "No message :(", "Helios Assert", MB_OK | MB_ICONERROR);
#endif
	}
}
