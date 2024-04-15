#pragma once
#include "ShaderProgram.h"

namespace Engine {
	class Material
	{
	public:
		void ApplyMaterial();

		Ref<ShaderProgram> GetProgram() { return m_Program; }
		void SetProgram(Ref<ShaderProgram> program) { m_Program = program; }

		void SetAlbedoTexture(const UUID& uuid) { m_AlbedoTexture = uuid; m_UseAlbedoTexture = true; }
		void SetNormalTexture(const UUID& uuid) { m_NormalTexture = uuid; m_UseNormalTexture = true; }
		void SetRoughnessTexture(const UUID& uuid) { m_RoughnessTexture = uuid; m_UseRoughnessTexture = true; }
		void SetMetalnessTexture(const UUID& uuid) { m_MetalnessTexture = uuid; m_UseMetalnessTexture = true; }
		void SetSpecularTexture(const UUID& uuid) { m_SpecularTexture = uuid; m_UseSpecularTexture = true; }
		void SetHeightTexture(const UUID& uuid) { m_HeightTexture = uuid; m_UseHeightTexture = true; }
		void SetOpacityTexture(const UUID& uuid) { m_OpacityTexture = uuid; m_UseOpacityTexture = true; }
		void SetAOTexture(const UUID& uuid) { m_AOTexture = uuid; m_UseAOTexture = true; }
		void SetRefractionTexture(const UUID& uuid) { m_RefractionTexture = uuid; m_UseRefractionTexture = true; }
		void SetEmissiveTexture(const UUID& uuid) { m_EmissiveTexture = uuid; m_UseEmissiveTexture = true; }

		UUID GetAlbedoTexture() const { return m_AlbedoTexture; }
		UUID GetNormalTexture() const { return m_NormalTexture; }
		UUID GetRoughnessTexture() const { return m_RoughnessTexture; }
		UUID GetMetalnessTexture() const { return m_MetalnessTexture; }
		UUID GetSpecularTexture() const { return m_SpecularTexture; }
		UUID GetHeightTexture() const { return m_HeightTexture; }
		UUID GetOpacityTexture() const { return m_OpacityTexture; }
		UUID GetAOTexture() const { return m_AOTexture; } 
		UUID GetRefractionTexture() const { return m_RefractionTexture; }
		UUID GetEmissiveTexture() const { return m_EmissiveTexture; }

		bool UsesAlbedoTexture() const { return m_UseAlbedoTexture; }
		bool UsesNormalTexture() const { return m_UseNormalTexture; }
		bool UsesRoughnessTexture() const { return m_UseRoughnessTexture; }
		bool UsesMetalnessTexture() const { return m_UseMetalnessTexture; }
		bool UsesSpecularTexture() const { return m_UseSpecularTexture; }
		bool UsesHeightTexture() const { return m_UseHeightTexture; }
		bool UsesOpacityTexture() const { return m_UseOpacityTexture; }
		bool UsesAOTexture() const { return m_UseAOTexture; } 
		bool UsesRefractionTexture() const { return m_UseRefractionTexture; }
		bool UsesEmissiveTexture() const { return m_UseEmissiveTexture; }

	protected:
		Ref<ShaderProgram> m_Program;

		UUID m_AlbedoTexture;
		bool m_UseAlbedoTexture = false;
		UUID m_NormalTexture;
		bool m_UseNormalTexture = false;
		UUID m_RoughnessTexture;
		bool m_UseRoughnessTexture = false;
		UUID m_MetalnessTexture;
		bool m_UseMetalnessTexture = false;
		UUID m_SpecularTexture;
		bool m_UseSpecularTexture = false;
		UUID m_HeightTexture;
		bool m_UseHeightTexture = false;
		UUID m_OpacityTexture;
		bool m_UseOpacityTexture = false;
		UUID m_AOTexture; // Ambient Occlusion
		bool m_UseAOTexture = false;
		UUID m_RefractionTexture;
		bool m_UseRefractionTexture = false;
		UUID m_EmissiveTexture;
		bool m_UseEmissiveTexture = false;
	};
}
