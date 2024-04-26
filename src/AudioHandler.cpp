#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#include "sndfile.hh"

#include <iostream>
#include <list>
#include <windows.h>
#include <mmsystem.h>
#include "AudioHandler.h"
#include <complex>
#include <fftw3.h>

using namespace std;

//~QP Original~

//For simultaneously playing and parsing audio files
AudioHandler::AudioHandler(void)
{
	now_playing = 0;
	is_playing = false;
	sample_buffer = NULL;
}

AudioHandler::~AudioHandler(void)
{

}

void AudioHandler::create(list<string> songs)
{
	song_list = {};
	for (std::list<string>::iterator skrelp = songs.begin(); skrelp != songs.end(); skrelp++)
	{
		song_list.push_back(*skrelp);
	}
}

void AudioHandler::play(int index)
{
	if (index >= 0 && index < song_list.size())
	{
		stop();
		now_playing = index;
		parse();

		wstring temp = std::wstring(song_list[now_playing].begin(), song_list[now_playing].end());
		if (PlaySoundW(temp.c_str(), NULL, SND_FILENAME /* | SND_NODEFAULT*/ | SND_ASYNC | SND_LOOP))
		{
			cout << "Now Playing: " + song_list[now_playing] << endl;
			is_playing = true;
		}
		else
		{
			cout << "Song at index " + std::to_string(now_playing) + " failed to load!" << endl;
		}
	}
}
void AudioHandler::stop()
{
	is_playing = false;
	PlaySound(0, 0, 0);
}

/// <summary>
/// Parse the audio data via libsndfile to fill the sample buffer
/// </summary>
void AudioHandler::parse() //implementation inspired by this tutorial: https://cindybui.me/pages/blogs/visual_studio_0#libsndfile
{
	snd = SndfileHandle(song_list[now_playing]/*, SFM_READ, SF_FORMAT_WAV, 2, 44100*/);
	sample_buffer = new float[snd.frames() * snd.channels()];
	snd.readf(sample_buffer, snd.frames());

	cout << "Channels: " + std::to_string(snd.channels()) << endl;
	cout << "Number of Frames: " + std::to_string(snd.frames()) << endl;
}

/// <summary>
/// 
/// </summary>
/// <param name="time">time (seconds) since the song began</param>
/// <param name="dt">delta time (seconds)</param>
/// <param name="lf"></param>
/// <param name="hf"></param>
/// <returns></returns>
bool AudioHandler::extractfft(float time, float dt, float &lf, float &hf)
{
	bool song_ending = false;
	unsigned int samples = 0;
	if (sample_buffer) {
		//if()
		for (int i = dt * 2 * 44100; i < snd.frames() * snd.channels() && i < (dt + 1) * 2 * 44100; i++)
		{
			samples++;
			//cout << "sample buffer contents: " + std::to_string(sample_buffer[i]) << endl;
			//if (sample_buffer[i] > 0) ampl += sample_buffer[i];

		}

		cout << "Samples at dt " + std::to_string(dt) + ": " + std::to_string(samples) << endl;


	}
	return song_ending;
}