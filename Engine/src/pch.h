#pragma once

#ifdef PLATFORM_WINDOWS
#ifndef NOMINMAX
// See github.com/skypjack/entt/wiki/Frequently-Asked-Questions#warning-c4003-the-min-the-max-and-the-macro
#define NOMINMAX
#endif
#endif

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

#include "Utils/Log.h"
#include "Utils/DataStructures.h"

#include "Core/UUID.h"

#include "Core.h"

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#undef near
#undef far
#endif
