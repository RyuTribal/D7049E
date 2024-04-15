#include "pch.h"
#include "Material.h"
#include "Renderer.h"
#include "Assets/TextureLibrary.h"

namespace Engine {
    void Material::ApplyMaterial()
    {
		m_Program->Activate();
#if 0
		HVE_CORE_TRACE_TAG("Materials",
			"Using the texture maps: Albedo: {0}, Normal: {1}, Roughness: {2}, Metalness: {3}, Specular: {4}, "
			"Height: {5}, Opacity: {6}, Ambient Occlusion: {7}, Refraction: {8}, Emissive: {9}",
			UsesAlbedoTexture() ? "Yes" : "No",
			UsesNormalTexture() ? "Yes" : "No",
			UsesRoughnessTexture() ? "Yes" : "No",
			UsesMetalnessTexture() ? "Yes" : "No",
			UsesSpecularTexture() ? "Yes" : "No",
			UsesHeightTexture() ? "Yes" : "No",
			UsesOpacityTexture() ? "Yes" : "No",
			UsesAOTexture() ? "Yes" : "No",
			UsesRefractionTexture() ? "Yes" : "No",
			UsesEmissiveTexture() ? "Yes" : "No"
		);
#endif
		if (UsesAlbedoTexture()) {
			TextureLibrary::Get()->GetTexture(m_AlbedoTexture)->Bind(0);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(0);
		}
		m_Program->UploadIntData("u_AlbedoTexture", 0);

		if (UsesNormalTexture()) {
			TextureLibrary::Get()->GetTexture(m_NormalTexture)->Bind(1);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(1);
		}
		m_Program->UploadIntData("u_NormalTexture", 1);

		if (UsesRoughnessTexture()) {
			TextureLibrary::Get()->GetTexture(m_RoughnessTexture)->Bind(2);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(2);
		}
		m_Program->UploadIntData("u_RoughnessTexture", 2);

		if (UsesMetalnessTexture()) {
			TextureLibrary::Get()->GetTexture(m_MetalnessTexture)->Bind(3);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(3);
		}
		m_Program->UploadIntData("u_MetalnessTexture", 3);

		if (UsesSpecularTexture()) {
			TextureLibrary::Get()->GetTexture(m_SpecularTexture)->Bind(4);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(4);		
		}
		m_Program->UploadIntData("u_SpecularTexture", 4);

		if (UsesEmissiveTexture())
		{
			TextureLibrary::Get()->GetTexture(m_EmissiveTexture)->Bind(5);
		}
		else
		{
			Renderer::Get()->GetDefaultMap()->Bind(5);
		}
		m_Program->UploadIntData("u_EmissiveTexture", 5);


		if (UsesHeightTexture()) {
			TextureLibrary::Get()->GetTexture(m_HeightTexture)->Bind(6);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(6);
		}
		m_Program->UploadIntData("u_HeightTexture", 6);

		if (UsesOpacityTexture()) {
			TextureLibrary::Get()->GetTexture(m_OpacityTexture)->Bind(7);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(7);
		}
		m_Program->UploadIntData("u_OpacityTexture", 7);

		if (UsesAOTexture()) {
			TextureLibrary::Get()->GetTexture(m_AOTexture)->Bind(8);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(8);
		}
		m_Program->UploadIntData("u_AOTexture", 8);

		if (UsesRefractionTexture()) {
			TextureLibrary::Get()->GetTexture(m_RefractionTexture)->Bind(9);
		}
		else {
			Renderer::Get()->GetDefaultMap()->Bind(9);
		}
		m_Program->UploadIntData("u_RefractionTexture", 9);


    }
}
