#include "pch.h"
#include "ScriptGlue.h"
#include <mono/metadata/assembly.h>
#include <mono/metadata/object.h>

namespace Engine {

#define HVE_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Engine.InternalCalls::" #Name, Name)

	static void NativeLog(MonoString* message, int parameter)
	{
		char* cstr = mono_string_to_utf8(message);
		std::string str(cstr);
		mono_free(cstr);
		std::cout << str << ", " << parameter << std::endl;
	}

	static void NativeLogVector(glm::vec3* parameter, glm::vec3* destination)
	{
		HVE_CORE_WARN("Value: {0}", *parameter);

		*destination = glm::cross(*parameter, glm::vec3(parameter->x, parameter->y, -parameter->z));
	}

	void ScriptGlue::RegisterFunctions()
	{
		HVE_ADD_INTERNAL_CALL(NativeLog);
		HVE_ADD_INTERNAL_CALL(NativeLogVector);
	}
}
