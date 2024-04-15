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
			TextureLibrary::Get()->GetTexture(m_AlbedoTexture)->Bind(6);
		}
		else {
			Renderer::GetGrayTexture()->Bind(6);
		}
		m_Program->UploadIntData("u_AlbedoTexture", 6);

		if (UsesNormalTexture()) {
			TextureLibrary::Get()->GetTexture(m_NormalTexture)->Bind(1);
		}
		else {
			Renderer::GetBlueTexture()->Bind(1);
		}
		m_Program->UploadIntData("u_NormalTexture", 1);

		if (UsesRoughnessTexture()) {
			TextureLibrary::Get()->GetTexture(m_RoughnessTexture)->Bind(2);
		}
		else {
			Renderer::GetGrayTexture()->Bind(2);
		}
		m_Program->UploadIntData("u_RoughnessTexture", 2);

		if (UsesMetalnessTexture()) {
			TextureLibrary::Get()->GetTexture(m_MetalnessTexture)->Bind(3);
		}
		else {
			Renderer::GetBlackTexture()->Bind(3);
		}
		m_Program->UploadIntData("u_MetalnessTexture", 3);

		if (UsesSpecularTexture()) {
			TextureLibrary::Get()->GetTexture(m_SpecularTexture)->Bind(7);
		}
		else {
			Renderer::GetWhiteTexture()->Bind(7);
		}
		m_Program->UploadIntData("u_SpecularTexture", 7);

		if (UsesEmissiveTexture())
		{
			TextureLibrary::Get()->GetTexture(m_EmissiveTexture)->Bind(5);
		}
		else
		{
			Renderer::GetBlackTexture()->Bind(5);
		}
		m_Program->UploadIntData("u_EmissiveTexture", 5);


		if (UsesHeightTexture()) {
			TextureLibrary::Get()->GetTexture(m_HeightTexture)->Bind(8);
		}
		else {
			Renderer::GetGrayTexture()->Bind(8);
		}
		m_Program->UploadIntData("u_HeightTexture", 8);

		if (UsesOpacityTexture()) {
			TextureLibrary::Get()->GetTexture(m_OpacityTexture)->Bind(11);
		}
		else {
			Renderer::GetWhiteTexture()->Bind(11);
		}
		m_Program->UploadIntData("u_OpacityTexture", 11);

		if (UsesAOTexture()) {
			TextureLibrary::Get()->GetTexture(m_AOTexture)->Bind(9);
		}
		else {
			Renderer::GetWhiteTexture()->Bind(9);
		}
		m_Program->UploadIntData("u_AOTexture", 9);

		if (UsesRefractionTexture()) {
			TextureLibrary::Get()->GetTexture(m_RefractionTexture)->Bind(10);
		}
		else {
			Renderer::GetBlackTexture()->Bind(10);
		}
		m_Program->UploadIntData("u_RefractionTexture", 10);


    }
}
