#ifndef SHADERCLASS_H
#define SHADERCLASS_H

#include <GL/glew.h>
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

//Original code was written by Prof. Chao Peng for IGME740 Assignment 3 project example,
//modified by Quinn Poyneer initially for Assignment 3 submission and
//subsequently for the Final Project of both IGME670 and IGME740

//this particular file remains wholly unmodified from the original created by Prof. Peng.

class ShaderClass
{
public:
	unsigned int id;
	//unsigned int vShader, fShader, cShader;
	//GLuint curAttributeLoc;
public:
	ShaderClass(void);
	~ShaderClass(void);
	void create(const char* shaderFileName, GLenum targetType);
	void destroy(); 

private:
	char* loadShaderFile(const char* fn); // load the shader text file
	void printShaderInfoLog(unsigned int shader_id); // print the shader info log to the console

};

#endif