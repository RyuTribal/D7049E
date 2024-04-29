#pragma once

#include "Base.h"
#include "Log.h"

#ifdef PLATFORM_WINDOWS
#define HVE_DEBUG_BREAK __debugbreak()
#elif defined(HVE_COMPILER_CLANG)
#define HVE_DEBUG_BREAK __builtin_debugtrap()
#else
#define HVE_DEBUG_BREAK
#endif

#ifdef DEBUG
#define HVE_ENABLE_ASSERTS
#endif

#define HVE_ENABLE_VERIFY

#ifdef HVE_ENABLE_ASSERTS
#ifdef HVE_COMPILER_CLANG
#define HVE_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Core, "Assertion Failed", ##__VA_ARGS__)
#define HVE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Assertion Failed", ##__VA_ARGS__)
#else
#define HVE_CORE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Core, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
#define HVE_ASSERT_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Assertion Failed" __VA_OPT__(,) __VA_ARGS__)
#endif

#define HVE_CORE_ASSERT(condition, ...) { if(!(condition)) { HVE_CORE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); HVE_DEBUG_BREAK; } }
#define HVE_ASSERT(condition, ...) { if(!(condition)) { HVE_ASSERT_MESSAGE_INTERNAL(__VA_ARGS__); HVE_DEBUG_BREAK; } }
#else
#define HVE_CORE_ASSERT(condition, ...)
#define HVE_ASSERT(condition, ...)
#endif

#ifdef HVE_ENABLE_VERIFY
#ifdef HVE_COMPILER_CLANG
#define HVE_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Core, "Verify Failed", ##__VA_ARGS__)
#define HVE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Verify Failed", ##__VA_ARGS__)
#else
#define HVE_CORE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Core, "Verify Failed" __VA_OPT__(,) __VA_ARGS__)
#define HVE_VERIFY_MESSAGE_INTERNAL(...)  ::Engine::Log::PrintAssertMessage(::Engine::Log::Type::Client, "Verify Failed" __VA_OPT__(,) __VA_ARGS__)
#endif

#define HVE_CORE_VERIFY(condition, ...) { if(!(condition)) { HVE_CORE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); HVE_DEBUG_BREAK; } }
#define HVE_VERIFY(condition, ...) { if(!(condition)) { HVE_VERIFY_MESSAGE_INTERNAL(__VA_ARGS__); HVE_DEBUG_BREAK; } }
#else
#define HVE_CORE_VERIFY(condition, ...)
#define HVE_VERIFY(condition, ...)
#endif
