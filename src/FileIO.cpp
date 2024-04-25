#define BOOST_FILESYSTEM_VERSION 3
#define BOOST_FILESYSTEM_NO_DEPRECATED
#include <boost>

#include "FileIO.h"

#include <iostream>
#include <fstream>
#include <list>
#include <vector>
#include <filesystem>

using namespace std;

//~QP Original*~
//*except some implementation bits, which were taken from this: https://stackoverflow.com/questions/11140483/how-to-get-list-of-files-with-a-specific-extension-in-a-given-folder

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

list<string> FileIO::filelookup(string ext)
{
	std::string path(directory);
	std::string ext(ext);

	//for(auto &p : 
}