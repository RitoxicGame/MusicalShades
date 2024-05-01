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

list<string> FileIO::filelookup(string ext)
{
	//makes heavy use of this implementation: https://stackoverflow.com/a/47975458

	//the filepath to search
	std::string path(directory);
	//the extension to look for
	std::string ext(ext);

	//the list to be returned
	list<string> files;

	for (auto& p : std::filesystem::recursive_directory_iterator(path))
	{
		if (p.path().extension() == ext)
			files.push_back(p.path().stem().string());
	}
	for (std::list<string>::iterator i = files.begin(); i != files.end(); i++)
		cout << *i << endl;
	return files;
}