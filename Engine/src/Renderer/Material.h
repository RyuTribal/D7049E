#pragma once
#include "ShaderProgram.h"

namespace Engine {
	class Material {
	public:
		virtual void ApplyMaterial() = 0;
		virtual std::string& GetName() = 0;

		Ref<ShaderProgram> GetProgram() { return m_Program; }

	protected:
		Ref<ShaderProgram> m_Program;
	};
}