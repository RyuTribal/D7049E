#pragma once

#include <stdint.h>


namespace Engine {
	enum class HEMotionType : uint8_t
	{
		Static,						///< Non movable
		Kinematic,					///< Movable using velocities only, does not respond to forces
		Dynamic,					///< Responds to forces as a normal physics object
	};

	static std::string FromMotionTypeToString(HEMotionType type)
	{
		switch (type)
		{
			case HEMotionType::Static:		return "Static";
			case HEMotionType::Dynamic:		return "Dynamic";
			case HEMotionType::Kinematic:	return "Kinematic";
		}
	}

	static HEMotionType FromStringToMotionType(const std::string& type_string)
	{
		if (type_string == "Static")                return HEMotionType::Static;
		if (type_string == "Dynamic")               return HEMotionType::Dynamic;
		if (type_string == "Kinematic")             return HEMotionType::Kinematic;
	}

	static std::vector<HEMotionType> HEMotionTypes{ HEMotionType::Static , HEMotionType::Kinematic, HEMotionType::Dynamic };
}




