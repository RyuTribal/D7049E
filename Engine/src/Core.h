#pragma once

#include <memory>
#include <filesystem>

#ifdef DEBUG
#define HVE_ENABLE_ASSERTS
#endif

#ifdef PLATFORM_WINDOWS
    #define THROW_NATIVE_ERROR() {__debugbreak();}
#endif
#ifdef PLATFORM_LINUX
    #define THROW_NATIVE_ERROR() {__builtin_trap();}
#endif

#define HVE_EXPAND_MACRO(x) x
#define HVE_STRINGIFY_MACRO(x) #x

#ifdef HVE_ENABLE_ASSERTS

#define INTERNAL_ASSERT_IMPL(type, check, msg, ...) { if(!(check)) { HVE##type##ERROR(msg, __VA_ARGS__); THROW_NATIVE_ERROR(); } }
#define INTERNAL_ASSERT_WITH_MSG(type, check, ...) INTERNAL_ASSERT_IMPL(type, check, "Assertion failed: {0}", __VA_ARGS__)
#define INTERNAL_ASSERT_NO_MSG(type, check) INTERNAL_ASSERT_IMPL(type, check, "Assertion '{0}' failed at {1}:{2}", HVE_STRINGIFY_MACRO(check), std::filesystem::path(__FILE__).filename().string(), __LINE__)

#define INTERNAL_ASSERT_GET_MACRO_NAME(arg1, arg2, macro, ...) macro
#define INTERNAL_ASSERT_GET_MACRO(...) HVE_EXPAND_MACRO( INTERNAL_ASSERT_GET_MACRO_NAME(__VA_ARGS__, INTERNAL_ASSERT_WITH_MSG, INTERNAL_ASSERT_NO_MSG) )

// Currently accepts at least the condition and one additional parameter (the message) being optional
#define HVE_ASSERT(...) HVE_EXPAND_MACRO( INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_, __VA_ARGS__) )
#define HVE_CORE_ASSERT(...) HVE_EXPAND_MACRO( INTERNAL_ASSERT_GET_MACRO(__VA_ARGS__)(_CORE_, __VA_ARGS__) )
#else
#define HVE_ASSERT(...)
#define HVE_CORE_ASSERT(...)
#endif

#define BIT(x) (1 << x)

#define THROW_CORE_ERROR(...)                          \
    {                                                  \
        HVE_CORE_ERROR("Runtime error: {0}", __VA_ARGS__); \
        std::abort();                                  \
    }

namespace Engine {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define EPSILON 1e-6
