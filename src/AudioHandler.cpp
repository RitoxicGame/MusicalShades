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

// Uses libsndfile for audio data handling: https://github.com/libsndfile/libsndfile
// Tutorial for some basic uses found here: https://cindybui.me/pages/blogs/visual_studio_0
//uses FFTW for DFT implementation: https://www.fftw.org


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
//common destructor
AudioHandler::~AudioHandler(void)
{

}
//initializes the song list
void AudioHandler::create(list<string> songs)
{
	song_list = {};
	for (std::list<string>::iterator skrelp = songs.begin(); skrelp != songs.end(); skrelp++)
	{
		song_list.push_back(*skrelp);
	}
}
int AudioHandler::nowPlaying()
{
	return now_playing;
}
float AudioHandler::getDuration()
{
	return duration;
}

//play the song at a given index
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
//stop playing audio
void AudioHandler::stop()
{
	is_playing = false;
	PlaySound(0, 0, 0);
}

/// <summary>
/// Parse the audio data via libsndfile to fill the sample buffer for the FFT
/// </summary>
void AudioHandler::parse() //implementation inspired by this tutorial: https://cindybui.me/pages/blogs/visual_studio_0#libsndfile
{
	delete sample_buffer; //delete whatever might have been in the sample buffer before
	snd = SndfileHandle(song_list[now_playing]);
	sample_buffer = new float[snd.frames() * snd.channels()];
	snd.readf(sample_buffer, snd.frames());

	duration = (float)snd.frames() / snd.samplerate();

	//cout << "Channels: " + std::to_string(snd.channels()) << endl;
	//cout << "Number of Frames: " + std::to_string(snd.frames()) << endl;
	//cout << "Total duration = " + std::to_string((int)(snd.frames() / (snd.samplerate() * 60)))
	//	+ ":" + std::to_string((int)((snd.frames() / snd.samplerate()) % 60)) << endl;
}

/// <summary>
/// Compute the FFT and overwrite the float values for average magnitudes of low and high frequency waves
/// Some parts were adapted from <a href="https://cindybui.me/pages/blogs/visual_studio_0#fftwDemocode">an online tutorial by Cindy Bui</a>
/// </summary>
/// <param name="time">= time (seconds) since the song began</param>
/// <param name="dt">= delta time (seconds)</param>
/// <param name="lf">= average magnitude of low-frequency waves, where freq is <i>between 27.5Hz (lowest piano note) and 138.59Hz (C3)</i></param>
/// <param name="hf">= average magnitude of high-frequency waves, where freq is <i>above 138.59Hz (C3) but no greater than 4186.01Hz (highest piano note)</i></param>
/// <returns>returns whether or not the song will end within one dT (fractions of a second, under normal circumstances)</returns>
bool AudioHandler::extractfft(float time, float dt, float &lf, float &hf)
{
	dt = min(dt, 1.0f); //failsafe in case program starts running slowly: limits update batches to 1 second. this shouldn't need to come into play outside of debugging

	bool song_ending = false; //whether or not the song is about to end

	unsigned int batch_size = 0; //the number of samples to compute in this run

	if (sample_buffer) { //can't read the samples if there <i>are</i> no samples XD
		if (time + dt < duration)
		{//only compute the whole batch if there are enough samples remaining to do so
			batch_size = snd.samplerate() * dt * snd.channels();
		}
		else
		{//otherwise, the batch size should be however many samples are left in the song
			song_ending = true;
			batch_size = floor((duration - time) * snd.samplerate() * snd.channels());
		}

		int padded_length = next_pow_2(batch_size); //this part's from the above tutorial. I don't know why she does it like this, but it can't hurt.
		in = fftwf_alloc_real((size_t)padded_length + 1);
		if (!in) return true; //if one of these things comes out null/fails to allocate, then the song's probably over.

		out = fftwf_alloc_complex(sizeof(fftwf_complex) * batch_size);
		if (!out) return true;

		plan = fftwf_plan_dft_r2c_1d(batch_size, in, out, FFTW_ESTIMATE); //libsndfile's readf() returns only real data, so we use a real-to-complex DFT
		if (!plan) return true; 

		for (int i = 0; i < batch_size; i += 1)//copy the batch from sample buffer to the input buffer
		{
			if ((int)(i + (time * snd.samplerate())) > snd.frames()) //array-index-oob protection (possibly redundant)
			{														 //not sure if this is necessary for a pointer, but I have it here just to be safe
				//inform Main that the song will end within one dT
				song_ending = true;
				break;
			}
			in[i] = sample_buffer[(int)(i + (time * snd.samplerate() * snd.channels()))];
		}
		for (int k = batch_size; k < padded_length + 1; k++)
		{
			in[k] = 0; //padding the input array. In retrospect, I think this might only be necessary for inline transforms, but oh well.
		}
		if(plan) fftwf_execute(plan);	//execute the FFT, if possible...
		else return song_ending;		//...otherwise, get out of here.
		
		float freq; //frequency of a given FFT output index
		float mag;	//magnitude of a given FFT output index

		//look at FFT results and update lf and hf accordingly
		for (int i = 0; i < floor(batch_size / 2) + 1; i++)

		 //for an input buffer of length N, a real-to-complex FFT will have N/2 + 1 (rounded down) indices.
		 //see also the documentation in section 2.3: One-Dimensional DFTs of Real Data 
		 //(begins on pg 12 of the pdf) https://www.fftw.org/fftw3.pdf

		 //special thanks to this guy https://youtu.be/3aOaUv3s8RY?si=_2QVU01SvJWt6xV3 
		 //for his videos https://youtu.be/rUtz-471LkQ?si=6SXXXYBIAnJ64_b6 on FFT analysis

		{
			//find the frequency of the indexed wave
			freq = (float)(i / (floor(batch_size / 2) + 1))*snd.samplerate();

			//calculate the magnitude of the wave via the pythagorean theorem
			mag = sqrtf(powf(std::real(out[i][0]), 2) + powf(std::imag(out[i][1]), 2));

			mag *= 1 + (abs(i - (floor(batch_size / 2) + 1)) / (floor(batch_size / 2) + 1)); //scale based on relative frequency

			//ignoring frequencies outside piano range (freq must be between 27.5Hz and 4186.01Hz)
			if (freq >= 27.5f && freq <= 138.59f) //anything at or below c3 is considered low frequency
			{
				lf += (int)mag; //cast as int to filter out any noise (i.e. anything with a magnitude < 1)
			}					//(most waves seem to give non-zero magnitudes even if they sound silent)
			else if(freq <= 4186.01)
			{
				hf += (int)mag; //add magnitudes to lf and hf
			}
		}

		lf /= floor(batch_size / 2) + 1; //get the average magnitude for low/high freq waves by dividing
		hf /= floor(batch_size / 2) + 1; //by output buffer size.

		//cout<< "lf: " << lf << "\t-- hf: " << hf << endl;

		//free up the memory allocated to the FFTW objects
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