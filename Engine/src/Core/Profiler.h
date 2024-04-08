#pragma once

#define HVE_ENABLE_PROFILING !DIST

#if HVE_ENABLE_PROFILING 
#include <tracy/Tracy.hpp>
#endif

#if HVE_ENABLE_PROFILING
#define HVE_PROFILE_MARK_FRAME			FrameMark;
// NOTE: Use HVE_PROFILE_FUNC ONLY at the top of a function
//				Use HVE_PROFILE_SCOPE / HVE_PROFILE_SCOPE_DYNAMIC for an inner scope
#define HVE_PROFILE_FUNC(...)			ZoneScoped##__VA_OPT__(N(__VA_ARGS__))
#define HVE_PROFILE_SCOPE(...)			HVE_PROFILE_FUNC(__VA_ARGS__)
#define HVE_PROFILE_SCOPE_DYNAMIC(NAME)  ZoneScoped; ZoneName(NAME, strlen(NAME))
#define HVE_PROFILE_THREAD(...)          tracy::SetThreadName(__VA_ARGS__)
#else
#define HVE_PROFILE_MARK_FRAME
#define HVE_PROFILE_FUNC(...)
#define HVE_PROFILE_SCOPE(...)
#define HVE_PROFILE_SCOPE_DYNAMIC(NAME)
#define HVE_PROFILE_THREAD(...)
#endif