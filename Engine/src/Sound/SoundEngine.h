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
		static void Stop(SoLoud::handle handle);

		static bool IsSoundPlaying(SoLoud::handle handle);

		static void SetLooping(SoLoud::handle handle, bool is_looping);
		static void SetVolume(SoLoud::handle handle, float volume);
	};
}


