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

class AudioHandler
{
public:
	vector<string> song_list;	//list of songs
	int now_playing;			//song index
	bool is_playing;			//is the thing paused?

public:
	AudioHandler(void);
	~AudioHandler(void);

	void create(list<string> songs);

	void play(int index);
	void stop();

//private:
//	void debug_print();
};