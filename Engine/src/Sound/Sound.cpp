#include <pch.h>
#include "Sound.h"

#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>
#include <iostream>

namespace Engine {
	Sound::Sound(std::string soundDirectory, float globalVolume)
	{
		m_soundPath = soundDirectory;
		//s_soloud->setGlobalVolume(globalVolume);
		if (!m_isInitialized)
		{
			s_soloud->init();
			m_isInitialized = true;
		}
	}

	Sound::~Sound()
	{
		//s_soloud->deinit();
		//m_isInitialized = false;
	}

	bool Sound::AddGlobalSound(const char* soundPath, bool looping)
	{
		if (m_soundfile.load(soundPath) != 0)
		{
			HVE_CORE_WARN("Could not find soundfile at path " + m_soundPath + soundPath);
			return false;
		}
		HVE_CORE_INFO("Loaded soundfile at path " + m_soundPath + soundPath);

		if (looping)
		{
			m_soundfile.setLooping(true);
		}
	}

	bool Sound::PlayGlobalSound(float relativeVolume)
	{

		/*if (m_soundfile.get)
		{
			return false;
		}*/

		m_playingSound = s_soloud->play(m_soundfile, relativeVolume);


		return true;

	}

	bool Sound::CheckFileExistance(std::string fileName)
	{
		struct stat buffer;
		return (stat(fileName.c_str(), &buffer) == 0);
	}

	void Sound::SetGlobalVolume(float globalVolume)
	{
		s_soloud->setGlobalVolume(globalVolume);
	}

	/*bool Sound::PlayBackgroundMusic(const char* soundName, float relativeVolume)
	{
		if (!CheckFileExistance(m_soundPath + soundName))
		{
			return false;
		}

		m_backgroundMusicFile.load((m_soundPath + soundName).c_str());
		m_backgroundMusicFile.setLooping(true);

		m_backgroundMusic = s_soloud->playBackground(m_backgroundMusicFile, relativeVolume);

		s_soloud->setProtectVoice(m_backgroundMusic, true);

		return true;
	}

	void Sound::ChangeBackgroundMusicVolume(float relativeVolume, float fadingTime)
	{
		if (fadingTime > 0.0f)
		{
			s_soloud->fadeVolume(m_backgroundMusic, relativeVolume, fadingTime);
		}
		else
		{
			s_soloud->setVolume(m_backgroundMusic, relativeVolume);
		}
	}*/

	float Sound::GetGlobalVolume()
	{
		// TODO: May need to be refactored in general since right now it might be the case that every component is a separate sound system. Not ideal.
		return s_soloud->getGlobalVolume();
	}

	int Sound::GetNumberOfPlayingSounds()
	{
		return s_soloud->getActiveVoiceCount();
	}

	const char* Sound::GetSoundFilename(int index)
	{
		/*if (m_soundfile.find(soundName) != m_soundfiles.end())
		{
			return m_soundfiles.begin()->first.c_str();
		}*/
		
		return "sound";
	}

	bool Sound::GetSoundLoopingStatus(const char* soundName)
	{
		return m_soundfile.SHOULD_LOOP;
	}

	SoLoud::Soloud* Sound::s_soloud = new SoLoud::Soloud();
}
