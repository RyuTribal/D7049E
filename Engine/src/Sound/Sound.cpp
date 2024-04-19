#include "pch.h"
#include "Sound.h"

#include <soloud_audiosource.h>

namespace Engine {
	Sound::Sound(std::string soundDirectory, float globalVolume) {
		m_soundPath = soundDirectory;
		s_soloud.setGlobalVolume(globalVolume);
		s_soloud.init();
	}

	Sound::~Sound()
	{
		s_soloud.deinit();
	}

	bool Sound::PlayGlobalSound(const char *soundName, float relativeVolume)
	{
		if (!CheckFileExistance(m_soundPath + soundName)) {
			return false;
		}

		SoLoud::Wav sound;
		sound.load((m_soundPath + soundName).c_str());

		m_playingSounds[soundName] = s_soloud.play(sound, relativeVolume);

		// Wait until sounds have finished
		/*while (s_soloud.getActiveVoiceCount() > 0)
		{
			SoLoud::Thread::sleep(100);
		}*/

		return true;

	}

	bool Sound::CheckFileExistance(std::string fileName)
	{
		struct stat buffer;
		return (stat(fileName.c_str(), &buffer) == 0);
	}

	void Sound::SetGlobalVolume(float globalVolume)
	{
		s_soloud.setGlobalVolume(globalVolume);
	}

	bool Sound::PlayBackgroundMusic(const char* soundName, float relativeVolume)
	{
		if (!CheckFileExistance(m_soundPath + soundName)) {
			return false;
		}

		SoLoud::Wav bgm;

		bgm.load((m_soundPath + soundName).c_str());
		bgm.setLooping(true);

		m_backgroundMusic = s_soloud.playBackground(bgm, relativeVolume);

		s_soloud.setProtectVoice(m_backgroundMusic, true);

		return true;
	}

	void Sound::ChangeBackgroundMusicVolume(float relativeVolume, float fadingTime)
	{
		if (fadingTime > 0.0f) {
			s_soloud.fadeVolume(m_backgroundMusic, relativeVolume, fadingTime);
		}
		else {
			s_soloud.setVolume(m_backgroundMusic, relativeVolume);
		}
	}

	float Sound::GetGlobalVolume()
	{
		// TODO: May need to be refactored in general since right now it might be the case that every component is a separate sound system. Not ideal.
		return s_soloud.getGlobalVolume();
	}

}
