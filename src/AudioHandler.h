#pragma once
#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#include "sndfile.hh"

#include <iostream>
#include <list>
#include <vector>
#include <windows.h>
#include <mmsystem.h>
#include <complex>
#include <fftw3.h>

using namespace std;

//~QP Original~

//For simultaneously playing and parsing audio files
class AudioHandler
{
public:
	vector<string> song_list;	//list of songs
	bool is_playing;			//is there a song playing?

private:
	float duration;				//song duration (seconds), extrapolated from frames/samplerate
	int now_playing;			//index of the song currently being played
	SndfileHandle snd;			//file handler for sample buffer extraction
	float* sample_buffer;		//pointer to sample buffer for song info
	fftwf_plan plan;			//FFTW plan holder
	float* in;					//input buffer for FFTW
	fftwf_complex *out;			//output buffer for FFTW

public:
	AudioHandler(void);
	~AudioHandler(void);

	void create(list<string> songs); //don't ask why it takes in a list but make a vector XD

	//vvv don't want other programs to be able to alter these values

	int nowPlaying();
	float getDuration();

	void play(int index);
	void stop();

	bool extractfft(float time, float dt, float& lf, float& hf);
	
private:
	void parse();

	int next_pow_2(int x);
};