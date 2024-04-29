#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#define _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING

#include "sndfile.hh"
#include <iostream>
#include <stdlib.h>
#include <list>
#include <windows.h>
#include <mmsystem.h>
#include "AudioHandler.h"
#include <math.h>
#include <complex.h>
#include <fftw3.h>

using namespace std;

//~QP Original~

//For simultaneously playing and parsing audio files
AudioHandler::AudioHandler(void)
{
	now_playing = 0;
	is_playing = false;
	duration = 0;
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
		if (PlaySoundW(temp.c_str(), NULL, SND_FILENAME /* | SND_NODEFAULT*/ | SND_ASYNC/* | SND_LOOP*/))
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
	delete sample_buffer;
	snd = SndfileHandle(song_list[now_playing]);
	sample_buffer = new float[snd.frames() * snd.channels()];
	snd.readf(sample_buffer, snd.frames());

	duration = (float)snd.frames() / snd.samplerate();

	cout << "Channels: " + std::to_string(snd.channels()) << endl;
	cout << "Number of Frames: " + std::to_string(snd.frames()) << endl;
	cout << "Total duration = " + std::to_string((int)(snd.frames() / (snd.samplerate() * 60)))
		+ ":" + std::to_string((int)((snd.frames() / snd.samplerate()) % 60)) << endl;
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
	dt = min(dt, 1.0f);
	bool song_ending = false;
	unsigned int batch_size = 0;
	//cout << "song_time thinks it is: " + std::to_string(time) + " / " + std::to_string(duration) << endl;
	if (sample_buffer) {
		if (time + dt < duration)
		{//only compute the whole batch if there are enough samples remaining to do so
			batch_size = snd.samplerate() * dt * snd.channels();
		}
		else
		{
			song_ending = true;
			batch_size = floor((duration - time) * snd.samplerate() * snd.channels());
		}
		//cout << "planning start" << endl;
		int padded_length = next_pow_2(batch_size);
		in = fftwf_alloc_real((size_t)padded_length + 1);
		if (!in) return true;
		out = fftwf_alloc_complex(sizeof(fftwf_complex) * batch_size);
		if (!out) return true;
		plan = fftwf_plan_dft_r2c_1d(batch_size, in, out, FFTW_ESTIMATE);
		if (!plan) return true;
		//cout << "planning end" << endl;
		int k = batch_size;
		for (int i = 0; i < batch_size; i += 1)//copy the batch from sample buffer to the input buffer
		{
			if ((int)(i + (time * snd.samplerate())) > snd.frames())
			{
				//cout << "Reached end of frame buffer at time " + std::to_string(time) + "\n" +
				//	"Final DeltaT = " + std::to_string((int)(i + (time * snd.samplerate())) - snd.frames()) << endl;
				song_ending = true;
				break;
			}
			in[i] = sample_buffer[(int)(i + (time * snd.samplerate() * snd.channels()))];
		}
		//cout << "\nChecking samples " + std::to_string((time * snd.samplerate() * snd.channels())) +
		//	" through " + std::to_string((time * snd.samplerate()) + batch_size - 1) + 
		//	" at time " + std::to_string(time) << endl;
		for (k = min(batch_size, k); k < padded_length + 1; k++)
		{
			in[k] = 0;
		}
		if(plan) fftwf_execute(plan);
		else return song_ending;

		float freq;
		float mag;
		for (int i = 0; i < floor(batch_size / 2) + 1; i++)
		{
			freq = ((float)i / (floor(batch_size / 2) + 1))*snd.samplerate();
			mag = sqrtf(powf(std::real(out[i][0]), 2) + powf(std::imag(out[i][1]), 2));
			//cout << "freq/mag " + std::to_string(i) + ": " + std::to_string(freq) + "/" + std::to_string(mag) << endl;
			//ignoring frequencies outside piano range
			if (freq >= 27.5f && freq <= 130.0f) //anything at or below c3 is considered low frequency
			{
				lf += (int)mag; //cast as ints to filter out any noise
			}					 //(most samples get non-zero magnitudes even if they sound silent)
			else if(freq <= 4186.01)
			{
				hf += (int)mag;
			}
		}
		//cout << "Low freq = " + std::to_string(lf) +
		//	"; High freq = " + std::to_string(lf) << endl;
		lf /= floor(batch_size / 2) + 1;
		hf /= floor(batch_size / 2) + 1;

		fftwf_destroy_plan(plan);
		fftw_free(in);
		fftw_free(out);
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