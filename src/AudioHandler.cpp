#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#include "sndfile.hh"

#include <iostream>
#include <list>
#include <windows.h>
#include <mmsystem.h>
#include "AudioHandler.h"

using namespace std;

//~QP Original~

AudioHandler::AudioHandler(void)
{
	now_playing = 0;
	is_playing = false;
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