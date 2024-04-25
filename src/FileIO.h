#pragma once
#include <iostream>
#include <list>
#include <vector>
#include <filesystem>

using namespace std;

//~QP Original*~
//*except some implementation bits, which were taken from this: https://stackoverflow.com/questions/11140483/how-to-get-list-of-files-with-a-specific-extension-in-a-given-folder

class FileIO
{
public:
private:
	string directory;

public:
	FileIO(void);
	~FileIO(void);

	void create(string dir);

	list<string> filelookup(string ext); //get a list of filenames of a given extension within the directory
};