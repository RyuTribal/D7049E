#pragma once

#include <string>
#include <map>

#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>

namespace Engine {
	class Sound
	{
	public:
		Sound(std::string soundDirectory = "Sounds/", float globalVolume = 1.0f);
		~Sound();
		bool PlayGlobalSound(const char* soundName, float relativeVolume = 1.0f); // Takes a sound filename (e.g. "pew.waw") and a volume relative to the global volume (e.g. 0.5 is half as loud and 2 twice as loud) and plays the sound. Returns false if sound not found.
		void SetGlobalVolume(float globalVolume); // 0.0 = Mute, 1.0 = Default
		bool PlayBackgroundMusic(const char* soundName, float relativeVolume = 1.0f); // Plays sound file with looping enabled and with priority so that it's not interrupted
		void ChangeBackgroundMusicVolume(float relativeVolume, float fadingTime = 0.0f); // Separate BGM volume control, relative to global volume. fadingTime allows fading volume for e.g. subtly lowering BGM during dialogs
		float GetGlobalVolume();
		int GetNumberOfPlayingSounds();
		static SoLoud::Soloud* s_soloud;

	private:
		std::string m_soundPath;

		bool m_isInitialized = false;

		std::map<std::string, SoLoud::handle> m_playingSounds;
		std::map < std::string, SoLoud::Wav> m_soundfiles;
		SoLoud::handle m_backgroundMusic;
		SoLoud::Wav m_backgroundMusicFile;
		bool CheckFileExistance(std::string fileName);
	};
}


