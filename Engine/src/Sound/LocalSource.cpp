#include "pch.h"

#include "Assets/AssetManager.h"
#include "LocalSource.h"
#include "SoundEngine.h"

namespace Engine {
	LocalSource::LocalSource(AssetHandle handle) : m_Handle(handle)
	{
		SetSound(handle);
	}
	LocalSource::~LocalSource()
	{
	}
	void LocalSource::SetSound(AssetHandle handle)
	{
		std::filesystem::path absolute_path = AssetManager::GetMetadata(handle).FilePath;
		absolute_path = Project::GetFullFilePath(absolute_path);
		if (m_SoundFile.load(absolute_path.string().c_str()) != 0)
		{
			HVE_CORE_WARN_TAG("Sound", "Could not find soundfile at path " + absolute_path.string());
			return;
		}
		HVE_CORE_INFO_TAG("Sound", "Loaded soundfile at path " + absolute_path.string());
		m_SoundAsset = handle;
	}
	void LocalSource::PlaySound(glm::vec3 speakerPosition, bool preview = false)
	{
		if (!SoundEngine::IsSoundPlaying(m_Handle))
		{
			m_Handle = SoundEngine::Play3D(m_SoundFile, m_Handle, speakerPosition);
			SoundEngine::SetLooping(m_Handle, preview ? false : m_Looping);
			SoundEngine::SetVolume(m_Handle, m_Volume);
			SoundEngine::SetAttenuation(m_Handle, m_Rolloff);
		}
	}
	void LocalSource::StopSound()
	{
		if (SoundEngine::IsSoundPlaying(m_Handle))
		{
			SoundEngine::Stop(m_Handle);
		}
	}
	bool LocalSource::IsPlaying()
	{
		return SoundEngine::IsSoundPlaying(m_Handle);
	}
	void LocalSource::SetVolume(float volume)
	{
		m_Volume = volume;
		if (SoundEngine::IsSoundPlaying(m_Handle))
		{
			SoundEngine::SetVolume(m_Handle, m_Volume);
		}
	}
	void LocalSource::SetRolloff(float rolloff)
	{
		m_Rolloff = rolloff;
		if (SoundEngine::IsSoundPlaying(m_Handle))
		{
			SoundEngine::SetAttenuation(m_Handle, m_Rolloff);
		}
	}
	void LocalSource::SetLooping(bool looping)
	{
		m_Looping = looping;
		if (SoundEngine::IsSoundPlaying(m_Handle))
		{
			SoundEngine::SetLooping(m_Handle, m_Looping);
		}
	}
}
