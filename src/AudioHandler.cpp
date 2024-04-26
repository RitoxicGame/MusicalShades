#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application

#include "sndfile.hh"

#include <iostream>
#include <list>
#include <windows.h>
#include <mmsystem.h>
#include "AudioHandler.h"

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
	//SF_INFO info = { 44100, 2, SF_FORMAT_WAV, 1, 0 };
	//if (sf_format_check(&info)) SNDFILE* file = sf_open(song_list[now_playing].c_str(), SFM_READ, &info);
	//else cout << "Invalid info!" << endl;

	snd = SndfileHandle(song_list[now_playing]/*, SFM_READ, SF_FORMAT_WAV, 2, 44100*/);
	sample_buffer = new float[snd.frames() * snd.channels()];
	snd.readf(sample_buffer, snd.frames());

	cout << "Channels: " + std::to_string(snd.channels()) << endl;
	cout << "Number of Frames: " + std::to_string(snd.frames()) << endl;
}

float AudioHandler::extract(int frame)
{
	float amp = 0.0f;
	unsigned int samples = 0;
	if (sample_buffer) {
		for (int i = frame * 2 * 44100; i < snd.frames() * snd.channels() && i < (frame + 1) * 2 * 44100; i++)
		{
			samples++;
			//cout << "sample buffer contents: " + std::to_string(sample_buffer[i]) << endl;
			if (sample_buffer[i] > 0) amp += sample_buffer[i];
		}
		cout << "Samples on frame #" + std::to_string(frame) + ": " + std::to_string(samples) << endl;
	}
	return (samples <= 0 ? -1 : amp / samples);
}