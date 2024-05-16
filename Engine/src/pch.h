#pragma once

#ifdef PLATFORM_WINDOWS
#ifndef NOMINMAX
// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif
#endif

#include <cmath>
#include <iostream>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <cassert>
#include <stdexcept>
#include <limits>
#include <fstream>

#include <string>
#include <sstream>
#include <cstring>
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <array>
#include <stack>

#include <chrono>
#include <future>

#include <glm/glm.hpp>
#include <Jolt/Jolt.h>

#include "Core/Base.h"
#include "Core/Log.h"
#include "Core/Profiler.h"
#include "Core/Timer.h"
#include "Core/Buffer.h"
#include "Core/UUID.h"
#include "Math/Math.h"

#ifdef PLATFORM_WINDOWS
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

#ifdef DEBUG 
#define JPH_ENABLE_ASSERTS
#endif
#include <Jolt/Jolt.h>
