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

private:
	SndfileHandle snd;
	float* sample_buffer;		//pointer to sample buffer for song info
	fftwf_plan plan;			//fftw plan holder
	fftwf_complex *in, *out;

public:
	AudioHandler(void);
	~AudioHandler(void);

	void create(list<string> songs);

	void play(int index);
	void stop();

	bool extractfft(float time, float dt, float &lf, float &hf);
	
private:
	void parse();

//	void debug_print();
};