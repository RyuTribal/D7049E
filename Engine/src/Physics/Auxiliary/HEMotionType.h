#pragma once

#include <stdint.h>

enum class HEMotionType : uint8_t
{
	Static,						///< Non movable
	Kinematic,					///< Movable using velocities only, does not respond to forces
	Dynamic,					///< Responds to forces as a normal physics object
};

