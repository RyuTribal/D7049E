#pragma once
#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>
#include "Assets/Asset.h"


namespace Engine {
	class GlobalSource
	{
	public:
		GlobalSource() = default;
		GlobalSource(AssetHandle handle);
		~GlobalSource();
		void SetSound(AssetHandle handle);
		void PlaySound(bool preview);
		void StopSound();
		bool IsPlaying();

		bool IsLooping() { return m_Looping; }
		float GetVolume() { return m_Volume; }
		const std::string& GetTitle() { return m_Title; }
		AssetHandle GetSoundAsset() { return m_SoundAsset; }

		void SetVolume(float volume);
		void SetLooping(bool looping);
		void SetTitle(const std::string& title) { m_Title = title; }

	private:
		std::string m_Title = "Untitled Sound";
		AssetHandle m_SoundAsset = 0;
		SoLoud::Wav m_SoundFile;
		SoLoud::handle m_Handle = 0;

		bool m_Looping = false;

		float m_Volume = 1.f;
	};
}
