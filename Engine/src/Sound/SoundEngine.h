#pragma once

#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>

namespace Engine {
	class SoundEngine
	{
	public:
		static void Init();
		static void Update();
		static void Shutdown();

		static SoLoud::handle Play(SoLoud::Wav& file, SoLoud::handle handle);
		static SoLoud::handle Play3D(SoLoud::Wav& file, SoLoud::handle handle, glm::vec3 speakerPosition);
		static void Stop(SoLoud::handle handle);
		static void StopAll();

		static bool IsSoundPlaying(SoLoud::handle handle);

		static void SetLooping(SoLoud::handle handle, bool is_looping);
		static void SetVolume(SoLoud::handle handle, float volume);
		static void SetAttenuation(SoLoud::handle handle, float rolloffFactor, unsigned int attenuationModel = 1); // The default attenuation model is inverse distance which is realistic, the others are niche and may not need to be added to the editor

		static void SetListenerPosition(glm::vec3 listenerPosition);
	};
}


