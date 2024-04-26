#pragma once
#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#include "sndfile.hh"

#include <iostream>
#include <list>
#include <vector>
#include <windows.h>
#include <mmsystem.h>

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

public:
	AudioHandler(void);
	~AudioHandler(void);

	void create(list<string> songs);

	void play(int index);
	void stop();

	float extract(int frame);

private:
	void parse();

//	void debug_print();
};