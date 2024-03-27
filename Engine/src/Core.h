#pragma once

#include <memory>

#ifdef DEBUG
#define ENABLE_ASSERTS
#endif

#ifdef PLATFORM_WINDOWS
    #define THROW_NATIVE_ERROR() {__debugbreak();}
#endif
#ifdef PLATFORM_LINUX
    #define THROW_NATIVE_ERROR() {__builtin_trap();}
#endif

#ifdef ENABLE_ASSERTS
#define CORE_ASSERT(x, ...) { if(!(x)) { CORE_ERROR("Assertion Failed: {0}", __VA_ARGS__); THROW_NATIVE_ERROR()}}
#else
#define CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define THROW_CORE_ERROR(...)                          \
    {                                                  \
        CORE_ERROR("Runtime error: {0}", __VA_ARGS__); \
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
