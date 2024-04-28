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
	int now_playing;			//song index
	bool is_playing;			//is the current song paused?
	float duration;				//song duration (s), extrapolated from frames/(samplerate*channels)

private:
	SndfileHandle snd;
	float* sample_buffer;		//pointer to sample buffer for song info
	fftwf_plan plan;			//FFTW plan holder
	float* in;					//input buffer for FFTW
	fftwf_complex *out;			//output buffer for FFTW

public:
	AudioHandler(void);
	~AudioHandler(void);

	void create(list<string> songs);

	void play(int index);
	void stop();

	bool extractfft(float time, float dt, float& lf, float& hf);
	
private:
	void parse();

	int next_pow_2(int x);

//	void debug_print();
};