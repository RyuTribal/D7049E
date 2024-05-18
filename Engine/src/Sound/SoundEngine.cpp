#include "pch.h"
#include "SoundEngine.h"

namespace Engine {

	struct SoundInstanceData
	{
		
	};

	static SoundInstanceData* s_Instance = nullptr;

	SoLoud::Soloud* SoLoudInstance = nullptr;

	void Engine::SoundEngine::Init()
	{
		s_Instance = new SoundInstanceData();
		SoLoudInstance = new SoLoud::Soloud();
		SoLoudInstance->init();
		SoLoudInstance->set3dListenerPosition(0.0, 0.0, 0.0);
	}
	void SoundEngine::Update()
	{
		SoLoudInstance->update3dAudio();
	}

	void SoundEngine::Shutdown()
	{
		SoLoudInstance->deinit();
		delete SoLoudInstance;
		delete s_Instance;
		s_Instance = nullptr;
	}

	SoLoud::handle Engine::SoundEngine::Play(SoLoud::Wav& file, SoLoud::handle handle)
	{
		if (!s_Instance || !SoLoudInstance) return 0;
		return SoLoudInstance->play(file);
	}

	SoLoud::handle Engine::SoundEngine::Play3D(SoLoud::Wav& file, SoLoud::handle handle, glm::vec3 speakerPosition)
	{
		if (!s_Instance || !SoLoudInstance) return 0;
		return SoLoudInstance->play3d(file, speakerPosition.x, speakerPosition.y, speakerPosition.z);
	}

	void SoundEngine::Stop(SoLoud::handle handle)
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->stop(handle);
		}
	}

	void SoundEngine::StopAll()
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->stopAll();
		}
	}

	bool SoundEngine::IsSoundPlaying(SoLoud::handle handle)
	{
		return SoLoudInstance->isValidVoiceHandle(handle);
	}

	void SoundEngine::SetLooping(SoLoud::handle handle, bool is_looping)
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->setLooping(handle, is_looping);
		}
	}

	void SoundEngine::SetVolume(SoLoud::handle handle, float volume)
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->setVolume(handle, volume);
		}
	}
	void SoundEngine::SetAttenuation(SoLoud::handle handle, float rolloffFactor, unsigned int attenuationModel)
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->set3dSourceAttenuation(handle, attenuationModel, rolloffFactor);
		}
	}
	void SoundEngine::SetListenerPosition(glm::vec3 listenerPosition)
	{
		SoLoudInstance->set3dListenerPosition(listenerPosition.x, listenerPosition.y, listenerPosition.z);
		Update();
	}
}
