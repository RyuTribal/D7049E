#pragma once
#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>
#include "Assets/Asset.h"


namespace Engine {
	class LocalSource
	{
	public:
		LocalSource() = default;
		LocalSource(AssetHandle handle);
		~LocalSource();
		void SetSound(AssetHandle handle);
		void PlaySound(glm::vec3 speakerPosition, bool preview);
		void StopSound();
		bool IsPlaying();

		bool IsLooping() { return m_Looping; }
		float GetVolume() { return m_Volume; }
		float GetRolloff() { return m_Rolloff; }
		const std::string& GetTitle() { return m_Title; }
		AssetHandle GetSoundAsset() { return m_SoundAsset; }

		void SetVolume(float volume);
		void SetRolloff(float rolloff);
		void SetLooping(bool looping);
		void SetTitle(const std::string& title) { m_Title = title; }

	private:
		std::string m_Title = "Untitled Sound";
		AssetHandle m_SoundAsset = 0;
		SoLoud::Wav m_SoundFile;
		SoLoud::handle m_Handle = 0;

		bool m_Looping = false;

		float m_Volume = 1.f;
		float m_Rolloff = 0.1f;
	};
}
