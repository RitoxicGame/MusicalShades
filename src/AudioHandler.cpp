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
	sample_buffer = NULL; //set these to null, since a song is currently not playing
	out = NULL;
	in = NULL;
	plan = NULL;
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
/// Parse the audio data via libsndfile to fill the sample buffer, then compute fft
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
/// Largely borrowed from this tutorial: https://cindybui.me/pages/blogs/visual_studio_0#fftwDemocode
/// </summary>
/// <param name="time">time (seconds) since the song began</param>
/// <param name="dt">delta time (seconds)</param>
/// <param name="lf">average magnitude of low-frequency (>=  waves</param>
/// <param name="hf"></param>
/// <returns></returns>
bool AudioHandler::extractfft(float time, float dt, float &lf, float &hf)
{
	bool song_ending = false;
	unsigned int batch_size = 0;
	if (sample_buffer) {
		if (44100 * (time + dt) > snd.frames())
		{//only compute the whole batch if there are enough samples remaining to do so
			batch_size = 44100 * 2 * dt;
			int padded_length = next_pow_2(batch_size);
			in = fftwf_alloc_real((size_t) padded_length + 1);
			out = fftwf_alloc_complex(sizeof(fftwf_complex) * batch_size);

			for (int i = time * 2 * 44100; i < (time + dt) * 2 * 44100; i += 2)
			{
				
			}
		}
		//cout << "Samples between " + std::to_string(time) + "and " + std::to_string(time+dt) + " seconds: " + std::to_string(batch_size) << endl;
	}
	return song_ending;
}

/// <summary>
/// Straight-up stole this from https://cindybui.me/pages/blogs/visual_studio_0#fftwDemocode: 
/// Takes in an integer and computes the next closest power of 2
/// </summary>
/// <param name="x">input integer</param>
/// <returns>next power of 2 closest to x</returns>
int AudioHandler::next_pow_2(int x) {
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x++;
	return x;
}