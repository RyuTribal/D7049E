#include <pch.h>
#include "Sound.h"

#include <soloud.h>
#include <soloud_thread.h>
#include <soloud_wav.h>
#include <soloud_audiosource.h>
#include <iostream>

namespace Engine {
	Sound::Sound()
	{
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

	bool Sound::AddGlobalSound(const char* soundPath)
	{
		if (m_soundfile.load(soundPath) != 0)
		{
			HVE_CORE_WARN_TAG("Sound", "Could not find soundfile at path " + std::string(soundPath));
			return false;
		}
		HVE_CORE_INFO_TAG("Sound", "Loaded soundfile at path " + std::string(soundPath));
		m_soundPath = std::string(soundPath);

		return true;
	}

	bool Sound::PlayGlobalSound(float relativeVolume)
	{

		if (m_soundPath == "No soundfile selected" || s_soloud->getVoiceFromHandle_internal(m_playingSound) != -1)
		{
			return false;
		}

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

	float Sound::GetGlobalVolume()
	{
		// TODO: Fix: Creating a new sound system resets the global volume, not ideal
		return s_soloud->getGlobalVolume();
	}

	int Sound::GetNumberOfPlayingSounds()
	{
		return s_soloud->getActiveVoiceCount();
	}

	const char* Sound::GetSoundFilename()
	{	
		return m_soundPath.c_str();
	}

	bool Sound::GetSoundLoopingStatus()
	{
		return m_looping;
	}

	void Sound::SetSoundLoopingStatus(bool loop)
	{
		m_soundfile.setLooping(loop);
		m_looping = loop;
	}

	void Sound::StopPlayingSound()
	{
		s_soloud->stop(m_playingSound);
	}

	float Sound::GetVolume()
	{
		if (m_soundPath == "No soundfile selected")
		{
			return 1.0;
		}
		else
		{
			return s_soloud->getVolume(m_playingSound);
		}
	}

	void Sound::SetVolume(float volume)
	{
		if (m_soundPath == "No soundfile selected")
		{
			return;
		}
		else
		{
			s_soloud->setVolume(m_playingSound, volume);
		}
	}

	SoLoud::Soloud* Sound::s_soloud = new SoLoud::Soloud();
}
