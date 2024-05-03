#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
//#include <boost>

#include "FileIO.h"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <filesystem>

using namespace std;

//~QP Original~*
//*except some implementation bits, which were taken from their respective sources (cited inline)

FileIO::FileIO(void)
{
	//...
}
FileIO::~FileIO(void)
{
	//...
}

void FileIO::create(string dir)
{
	directory = dir;
}

/// <summary>
/// General-purpose file lookup function: iterates through all the files in 
/// <member name ="directory">the designated directory</member>
/// </summary>
/// <param name="ext">= The extension to look for (e.g. <i>".wav"</i>, <i>".obj"</i>, etc.)</param>
/// <returns>returns a string list containing all the file names whose extensions match <i>ext</i></returns>
list<string> FileIO::filelookup(string ext)
{
	//makes heavy use of this implementation: https://stackoverflow.com/a/47975458

	//the filepath to search
	std::string path(directory);

	//the list to be returned
	list<string> files;

	for (auto& p : std::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
			files.push_back(directory + "sounds\\" + p.path().stem().string());
	}
	return files;
}