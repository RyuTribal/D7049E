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

	void SoundEngine::Stop(SoLoud::handle handle)
	{
		if (s_Instance && SoLoudInstance)
		{
			SoLoudInstance->stop(handle);
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
}
