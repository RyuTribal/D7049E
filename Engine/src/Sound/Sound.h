#pragma once

#include <soloud_wav.h>

namespace SoLoud {
	class Soloud;
	class Wav;
	typedef unsigned int handle;
}


namespace Engine {
	class Sound
	{
	public:
		Sound();
		~Sound();
		bool AddGlobalSound(const char* soundPath); //Takes a filepath to a sound of .wav, .ogg or .mp3 format and adds it as the soundfile played by the component. Returns false if file not found.
		bool PlayGlobalSound(float relativeVolume = 1.0f); //Plays the components sound at a volume relative to the global volume (e.g. 0.5 is half as loud and 2 twice as loud). Returns false if no sound has been loaded. Will only play one sound instance at the same time.
		void SetGlobalVolume(float globalVolume); // 0.0 = Mute, 1.0 = Default
		float GetGlobalVolume();
		int GetNumberOfPlayingSounds(); //Gets the total number of sounds playing over all components.
		const char* GetSoundFilename();
		bool GetSoundLoopingStatus();
		void SetSoundLoopingStatus(bool loop); //A looping sound will play again and again from the beginning until stopped. Can't be changed while the sound is playing.
		void StopPlayingSound(); //Stops playing sound if the component is currently playing one, will start the sound from the beginning when next played.
		float GetVolume();
		void SetVolume(float volume); //Sets the volume relative to the global volume, can be changed while the sound is playing.
		static SoLoud::Soloud* s_soloud;

	private:
		std::string m_soundPath = "No soundfile selected";
		SoLoud::handle m_playingSound;
		SoLoud::Wav m_soundfile;

		bool m_looping = false;

		bool m_isInitialized = false;

		bool CheckFileExistance(std::string fileName);
	};
}


