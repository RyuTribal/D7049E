#include "pch.h"
#include "Phong.h"
#include "Renderer/Renderer.h"

namespace Engine{
	Phong::Phong()
	{
		std::string path = std::string(ROOT_PATH)+"/shaders/phong_material_shader";
		m_Program = CreateRef<ShaderProgram>(path);
	}

	void Phong::ApplyMaterial()
	{
		m_Program->Activate();
		m_Program->UploadVec3FloatData("u_CameraPos", Renderer::Get()->GetCamera()->CalculatePosition());
		m_Program->UploadVec3FloatData("u_Material.ambient", m_Ambient);
		m_Program->UploadVec3FloatData("u_Material.diffuse", m_Diffuse);
		m_Program->UploadVec3FloatData("u_Material.specular", m_Specular);
		m_Program->UploadFloatData("u_Material.shininess", m_Shininess);
	}
}