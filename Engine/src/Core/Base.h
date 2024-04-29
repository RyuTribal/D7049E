#pragma once

#include <memory>
#include <filesystem>

#ifdef DEBUG
#define HVE_ENABLE_ASSERTS
#endif

#if defined(__clang__)
#define HVE_COMPILER_CLANG
#elif defined(_MSC_VER)
#define HVE_COMPILER_MSVC
#endif

#ifdef HVE_COMPILER_MSVC
#define HVE_FORCE_INLINE __forceinline
#elif defined(HVE_COMPILER_CLANG)
#define HVE_FORCE_INLINE __attribute__((always_inline)) inline
#else
#define HVE_FORCE_INLINE inline
#endif

#ifdef PLATFORM_WINDOWS
    #define THROW_NATIVE_ERROR() {__debugbreak();}
#endif
#ifdef PLATFORM_LINUX
    #define THROW_NATIVE_ERROR() {__builtin_trap();}
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)
#define EPSILON 1e-6

#include "Core/Assert.h"

namespace Engine {

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	using byte = uint8_t;

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

}
