// This OpenGL project demonstrates a simple interactive camera implementation and a few vertex and fragment shaders. 
// Two modes of camera controls (press c to switch between two modes): 
// 1. Focus Mode: Holding ALT and Left Mouse Button (LMB) and moving the mouse will rotate the camera about the LookAt Position
//                Holding ALT and MMB and moving the mouse will pan the camera.
//                Holding ALT and RMB and moving the mouse will zoom the camera.
// 2. First-Person Mode: Pressing W, A, S, or D will move the camera
//                       Holding LMB and moving the mouse will roate the camera.
// Basic shader - demonstrate the basic use of vertex and fragment shader
// Displacement shader - a special fireball visual effects with Perlin noise function
// Toon shading shader - catoonish rendering effects
// Per-vertex shading v.s. per-fragment shading = visual comparison between two types of shading 

#pragma comment(lib, "winmm.lib") //https://stackoverflow.com/questions/9961949/playsound-in-c-console-application
#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/freeglut.h>
#endif
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/constants.hpp>
#include <glm/gtx/string_cast.hpp>

#include "Camera.h"
#include "Text.h"
#include "Mesh.h"
#include "AudioHandler.h"
#include "sndfile.hh"

#include <iostream>
#include <windows.h>
#include <mmsystem.h>
#include <chrono>
#include <ctime>
//#define _USES_MATH_DEFINES //pulled from this source: https://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c
//#include <math.h>
using namespace std;
using namespace glm;

// Original code was written by Prof. Chao Peng for IGME740 Assignment 3 project example,
// modified by Quinn Poyneer initially for Assignment 3 submission and
// subsequently for the Final Project of both IGME670 and IGME740

int g_winWidth  = 1024;
int g_winHeight = 512;

Camera g_cam;
Text g_text;

int text_display_select;

enum ScrollSpeed { slowscroll, medscroll, fastscroll };
enum OrbitSpeed { sloworbit, medorbit, fastorbit, ultraorbit, stoporbit };
enum NoiseLevels { nonoise, lownoise, mednoise, highnoise }; //dummied out
enum TextDisplay { tut1, tut2, lights, mesh };

unsigned char g_keyStates[256];

char v_shader_file[] =
".\\shaders\\per_Vertex_CUSTOM.vert";	// CUSTOM phong + displacement for warping a sphere in time with music


char f_shader_file[] =
".\\shaders\\per_Vertex_CUSTOM.frag";	// CUSTOM phong + displacement for warping a sphere in time with music

const char meshFile[128] = //technically works with the other meshes, but it's not quite as aesthetically pleasing
"Mesh/sphere.obj";
//"Mesh/bunny2K.obj";
//"Mesh/teapot.obj";
//"Mesh/teddy.obj";

Mesh g_mesh;

chrono::steady_clock::time_point oldTime; //clock/timer(s) used for FFT batch sizing
chrono::duration<double> elapsed;
chrono::steady_clock::time_point origin_time; //steady_clock reading after initialization() is called
chrono::duration<double> total_time; //current steady_clock reading: used for perlin noise calcs
double dT = 0.0; //delta time/deltaT, used for FFT

const float THETA_0 = pi<float>() / 4.0f; //light 0 (yellow), about y
float theta = THETA_0;

const float THETAALT_0 = pi<float>() / 6.0f; //light 1 (magenta), about x
float thetaAlt = THETAALT_0;

const float THETAALT2_0 = pi<float>() / 2.0f; //light 2 (cyan), about z
float thetaAlt2 = THETAALT2_0;

const float THETAALT3_0 = 5 * pi<float>() / 4.0f; //light 3 (blue), about y (#2)
float thetaAlt3 = THETAALT3_0;

const float THETAALT4_0 = 7 * pi<float>() / 6.0f; //light 4 (green), about x (#2)
float thetaAlt4 = THETAALT4_0;

const float THETAALT5_0 = 3 * pi<float>() / 2.0f; //light 5 (red), about z (#2)
float thetaAlt5 = THETAALT5_0;

//float deltaT; //old way of doing time calcs based on glut's timer. turned out to be less precise, so I nixed it
//float oldT = 0;

//base orbit radius
const float MIN_ORBIT_RAD = 7.0f;
float orbitrad = MIN_ORBIT_RAD;

//divisors for orbit speed -- higher number = slower orbit (orbit speed 0 is accounted for in the enum)
const float ORBIT_SPD_SLOW = 1.200;
const float ORBIT_SPD_MED = 0.600;
const float ORBIT_SPD_FAST = 0.300;
const float ORBIT_SPD_ULTRA = 0.150;

float orbitSpeedDenom;

//position for light 0 (yellow), orbiting y axis
vec3 g_lightPos = vec3(
	orbitrad * std::cos(theta), 
	0.0f, 
	orbitrad * std::sin(theta));

//position for light 3 (blue), orbiting y axis
vec3 g_lightPosAlt3 = vec3(
	orbitrad * std::cos(thetaAlt3), 
	0.0f, 
	orbitrad * std::sin(thetaAlt3));

//position for light 1 (magenta), orbiting x axis
vec3 g_lightPosAlt = vec3(
	0.0f, 
	orbitrad * std::sin(thetaAlt), 
	orbitrad * std::cos(thetaAlt));

//position for light 4 (green), orbiting x axis
vec3 g_lightPosAlt4 = vec3(
	0.0f, 
	orbitrad * std::sin(thetaAlt4), 
	orbitrad * std::cos(thetaAlt4));

//position for light 2 (cyan), orbiting z axis
vec3 g_lightPosAlt2 = vec3(
	orbitrad * std::cos(thetaAlt2), 
	orbitrad * std::sin(thetaAlt2), 
	0.0f);

//position for light 5 (red), orbiting z axis
vec3 g_lightPosAlt5 = vec3(
	orbitrad * std::cos(thetaAlt5), 
	orbitrad * std::sin(thetaAlt5), 
	0.0f);

int swap_lights = 0; //which light is currently selected

AudioHandler ah;	//audio handler -- holds song list, plays music, and parses song data
float low_freq;		//average low-frequency magnitude
float high_freq;	//average high-frequency magnitude
float basehf;		//unmodified high-frequency magnitude, for display purposes
int d_threshold = 1;//distortion control threshold, dec and inc by pressing '-' or '='
int d_boost = 1;	//distortion boost, dec and inc with '[' or ']'

double songT = 0;	//time since the song began -- this is a double to match precision with the clock
bool song_ending;	//if the song will end within the next few ms

float song_time_period = 0;	//tracks period for extracting fft data
const float STP = 0.04f; //how often (in seconds) to extract an FFT batch

//int song_menu_index_offset;

//float g_time = 0.0f; //dummied this out b/c the steady_clock is more precise

void initialization() 
{    
    g_cam.set(3.0f, 4.0f, 14.0f, 0.0f, 1.0f, -0.5f, g_winWidth, g_winHeight);
	g_text.setColor(0.0f, 0.0f, 0.0f);
	
	orbitSpeedDenom = ORBIT_SPD_SLOW; //default to slow orbit speed

	origin_time = chrono::steady_clock::now();
	oldTime = chrono::steady_clock::now();

	ah.create(
		{
			"sounds\\Rabi-Ribi Original Soundtrack - 45 No Remorse.wav",
			"sounds\\07 Huujirareta Youkai.wav",
			"sounds\\06 Mannennokigasa ni gochuui wo.wav",
			"sounds\\18 Mahoushoujotachi no hyakunensai.wav",
			"sounds\\25 Gouyoku na kemono no Memento (Arr.wav",
			"sounds\\Necromantic.wav",
			"sounds\\51_Song of Lament.wav",
			"sounds\\39 Eien no shunmu.wav",
			"sounds\\43 Yoru ga oritekuru ~ Evening Star.wav",
			"sounds\\32 Ningyousaiban _ U2 Akiyama.wav",
			"sounds\\34 Voile mahoutoshokan _ U2 Akiyama.wav",
			"sounds\\08 Neko Miko Reimu A.wav",
			"sounds\\18 Koi no Hyoketsu Otenba Yukemuri C.wav"
		});

	mat4 m = translate(mat4(1.0), vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, 3.14159f, vec3(0.0f, 1.0f, 0.0f));
	g_mesh.modelMat = scale(m, vec3(0.5f, 0.5f, 0.5f));
	g_mesh.create(meshFile, v_shader_file, f_shader_file);

	// add any stuff you want to initialize ...
}

/****** GL callbacks ******/
void initialGL()
{    
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f);
	
	glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
	
	glMatrixMode (GL_MODELVIEW);
	glLoadIdentity();
}

void idle()
{
    // add any stuff to update at runtime ....

    g_cam.keyOperation(g_keyStates, g_winWidth, g_winHeight);

	glutPostRedisplay();
}

//update the positions and angles of the lights
void setLightPos()
{
	//setting up light sources to orbit the origin
	//could've probably used gl rotate methods for this, but this was more intuitive (and more fun) for me

	//adjust orbit radius based on the low frequency samples' average magnitude within the last given song_time_perod
	orbitrad = MIN_ORBIT_RAD * (1 + (low_freq * (20 * (-1 * powf(song_time_period, 2) + STP))));

	if (orbitSpeedDenom > 0) //only change the angles if the lights are moving (also avoid dividing by 0)
	{
		theta += ((pi<float>() * dT) / orbitSpeedDenom);
		thetaAlt += ((pi<float>() * dT) / orbitSpeedDenom);
		thetaAlt2 += ((pi<float>() * dT) / orbitSpeedDenom);
		thetaAlt3 += ((pi<float>() * dT) / orbitSpeedDenom);
		thetaAlt4 += ((pi<float>() * dT) / orbitSpeedDenom);
		thetaAlt5 += ((pi<float>() * dT) / orbitSpeedDenom);
	}

	if (theta >= (2 * pi<float>())) theta -= (2 * pi<float>()); //clamp angle values to [0, 2*pi]
	if (thetaAlt >= (2 * pi<float>())) thetaAlt -= (2 * pi<float>());
	if (thetaAlt2 >= (2 * pi<float>())) thetaAlt2 -= (2 * pi<float>());
	if (thetaAlt3 >= (2 * pi<float>())) thetaAlt3 -= (2 * pi<float>());
	if (thetaAlt4 >= (2 * pi<float>())) thetaAlt4 -= (2 * pi<float>());
	if (thetaAlt5 >= (2 * pi<float>())) thetaAlt5 -= (2 * pi<float>());

	g_lightPos.x = orbitrad * std::cos(theta);
	g_lightPos.z = orbitrad * std::sin(theta);

	g_lightPosAlt.y = orbitrad * std::sin(thetaAlt);
	g_lightPosAlt.z = orbitrad * std::cos(thetaAlt);

	g_lightPosAlt2.x = orbitrad * std::cos(thetaAlt2);
	g_lightPosAlt2.y = orbitrad * std::sin(thetaAlt2);

	g_lightPosAlt3.x = orbitrad * std::cos(thetaAlt3);
	g_lightPosAlt3.z = orbitrad * std::sin(thetaAlt3);

	g_lightPosAlt4.y = orbitrad * std::sin(thetaAlt4);
	g_lightPosAlt4.z = orbitrad * std::cos(thetaAlt4);

	g_lightPosAlt5.x = orbitrad * std::cos(thetaAlt5);
	g_lightPosAlt5.y = orbitrad * std::sin(thetaAlt5);
}

//draw all of the lights
void drawLights()
{
	glPushMatrix();
	glLoadMatrixf(value_ptr(g_cam.viewMat)); //drawing lights in camera space
	glPushMatrix(); //lights are all drawn relative to the origin
	glPushMatrix();
	glPushMatrix();
	glPushMatrix();
	glPushMatrix();

	glColor3f(1, 1, 0); //set up light 0 (yellow, orbit y)
	glTranslatef(g_lightPos.x, g_lightPos.y, g_lightPos.z);
	(swap_lights == 0 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();
	glColor3f(1, 0, 1); //set up light 1 (magenta, orbit x)
	glTranslatef(g_lightPosAlt.x, g_lightPosAlt.y, g_lightPosAlt.z);
	(swap_lights == 1 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();
	glColor3f(0, 1, 1); //set up light 2 (cyan, orbit z)
	glTranslatef(g_lightPosAlt2.x, g_lightPosAlt2.y, g_lightPosAlt2.z);
	(swap_lights == 2 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();
	glColor3f(0, 0, 1); //set up light 3 (blue, orbit y)
	glTranslatef(g_lightPosAlt3.x, g_lightPosAlt3.y, g_lightPosAlt3.z);
	(swap_lights == 3 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();
	glColor3f(0, 1, 0); //set up light 4 (green, orbit x)
	glTranslatef(g_lightPosAlt4.x, g_lightPosAlt4.y, g_lightPosAlt4.z);
	(swap_lights == 4 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();
	glColor3f(1, 0, 0); //set up light 5 (red, orbit z)
	glTranslatef(g_lightPosAlt5.x, g_lightPosAlt5.y, g_lightPosAlt5.z);
	(swap_lights == 5 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix(); //set matrix to whatever is was before the lights were drawn
}

//dislpay the text
void drawText()
{
	int text_row = 1;
	string str = "";
	if (g_cam.isFocusMode()) {
		str = "Cam mode: Focus";
	}
	else if (g_cam.isFPMode()) {
		str = "Cam mode: FP";
	}
	g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	text_row++;

	str = "Song Time Elapsed: " + (ah.is_playing && !song_ending
		?
		std::to_string((int)songT / 60) + ":" +
		((int)(songT / 10) % 10 == 0 ? "0" : "") +
		std::to_string((int)songT % 60)
		:
		"---:--");
	g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	text_row++;

	str = "Song Duration: " + (ah.is_playing && !song_ending
		?
		std::to_string((int)ah.getDuration() / 60) + ":" + 
		((int)(ah.getDuration() / 10) % 10 == 0 ? "0" : "") +
		std::to_string((int)ah.getDuration() % 60)
		:
		"---:--");
	g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	text_row++;

	str = "Current BGM: " + (ah.is_playing && !song_ending ? ah.song_list[ah.nowPlaying()] : "N/A");
	g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	text_row++;

	switch (text_display_select)
	{
	default:
		break;

	case tut1:
		if (g_cam.isFPMode())
		{
			str = "Left Click to rotate, G/J to strafe, Y/H to move Forward/Backward";
		}
		else
		{
			str = "ALT+Left Click to rotate, ALT+Middle Mouse to pan, Scroll to zoom";
		}
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Right Click to open menu";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "ESC to exit program";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Press \"-\" / \"=\" to decrement/increment distortion threshold";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Press \"[\" / \"]\" to lower/boost distortion (see value in Mesh text display)";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Press \"q\" to reset light positions";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Press \"c\" to change camera modes";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		break;

	case tut2:
		str = "This program uses FFTW and libsndfile to deform a mesh in real time based on sound file data.";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "This is accomplished by first performing a DFT on a batch of samples, then";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "passing the the constituent waves' average magnitude values into a vertex shader.";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "High frequencies (4186.01Hz >= x > 138.59Hz) distort the mesh";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Low frequencies (138.59Hz >= x >= 27.5Hz) affect the lights' orbit";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Distortion threshold (arbitrary) clamps distortion asymptotically (min 4, max 20)";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Distortion boost (arbitrary) increases distortion magnitude (min 1, max 20)";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		break;

	case lights:
		str = "Displace lights (+/-) across their axes of orbit with letter keys:";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "y = R/F, x = D/A, z = S/W";
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Currently selected light: " + std::to_string(swap_lights);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Light " + std::to_string(swap_lights) + " position: (";
		switch (swap_lights)
		{
		case 0:
			str += std::to_string(g_lightPos.x) + ", "
				+ std::to_string(g_lightPos.y) + ", "
				+ std::to_string(g_lightPos.z) + ")";
			break;
		case 1:
			str += std::to_string(g_lightPosAlt.x) + ", "
				+ std::to_string(g_lightPosAlt.y) + ", "
				+ std::to_string(g_lightPosAlt.z) + ")";
			break;
		case 2:
			str += std::to_string(g_lightPosAlt2.x) + ", "
				+ std::to_string(g_lightPosAlt2.y) + ", "
				+ std::to_string(g_lightPosAlt2.z) + ")";
			break;
		case 3:
			str += std::to_string(g_lightPosAlt3.x) + ", "
				+ std::to_string(g_lightPosAlt3.y) + ", "
				+ std::to_string(g_lightPosAlt3.z) + ")";
			break;
		case 4:
			str += std::to_string(g_lightPosAlt4.x) + ", "
				+ std::to_string(g_lightPosAlt4.y) + ", "
				+ std::to_string(g_lightPosAlt4.z) + ")";
			break;
		case 5:
			str += std::to_string(g_lightPosAlt5.x) + ", "
				+ std::to_string(g_lightPosAlt5.y) + ", "
				+ std::to_string(g_lightPosAlt5.z) + ")";
			break;
		}
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "Theta " + std::to_string(swap_lights) + " value: ";
		switch (swap_lights)
		{
		case 0:
			str += std::to_string(theta);
			break;
		case 1:
			str += std::to_string(thetaAlt);
			break;
		case 2:
			str += std::to_string(thetaAlt2);
			break;
		case 3:
			str += std::to_string(thetaAlt3);
			break;
		case 4:
			str += std::to_string(thetaAlt4);
			break;
		case 5:
			str += std::to_string(thetaAlt5);
			break;
		}
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Orbit radius: " + std::to_string(orbitrad);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;

		str = "Delta Theta (rads per draw call): " + (orbitSpeedDenom != 0 ? std::to_string((pi<float>() * dT) / orbitSpeedDenom) : "N/A");
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		break;

	case mesh:
		str = "Vertex count: " + std::to_string(g_mesh.vert_num);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "Triangle count: " + std::to_string(g_mesh.tri_num);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "Distortion threshold (min 4, max 20): " + std::to_string(d_threshold+3);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "Distortion boost (min 1, max 20): " + std::to_string(d_boost);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "High-frequency portion: " + std::to_string(basehf);
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		str = "High-frequency portion (modified): " + std::to_string(((12.0f * d_threshold) / pi<float>()) * std::atanf((d_boost / (4.0f * d_threshold)) * basehf));
		g_text.draw(10, text_row * 15, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
		text_row++;
		break;
	}
}

void display()
{	 
	glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 mvp = g_cam.projMat * g_cam.viewMat;

	//setting up deltaT vars
	total_time = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - origin_time);
	elapsed = chrono::duration_cast<chrono::duration<double>>(chrono::steady_clock::now() - oldTime);
	oldTime = chrono::steady_clock::now();

	dT = elapsed.count();

	setLightPos();

	if (ah.is_playing && !song_ending) //if the song is playing and isn't about to end, 
	{
		
		song_time_period += dT;
		//song_time += deltaT;
		if (song_time_period > STP)
		{
			low_freq = 0;
			high_freq = 0;

			//song will end within approximately ten draw calls?
			song_ending = ah.extractfft((float)songT, song_time_period, low_freq, high_freq);

			basehf = high_freq;

			//cout << "Low freq avg m = " + std::to_string(low_freq) +
			//	"; High freq avg m = " + std::to_string(high_freq) << endl;

			high_freq = ((12.0f*d_threshold)/ pi<float>()) * std::atanf((d_boost/(4.0f*d_threshold)) * high_freq); //limits noise based on user input

			songT += song_time_period;
			song_time_period = 0;
		}
	}
	else if(songT > 0)
	{
		//cout << "Song ended at time: " + std::to_string(songT) + " / " + std::to_string(ah.getDuration()) << endl;
		songT = 0;
		high_freq = 0;
		low_freq = 0;
		song_time_period = 0;//STP;
	}

	// add any stuff you'd like to draw	

	glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);

	drawLights();

	g_cam.drawGrid();
    g_cam.drawCoordinateOnScreen(g_winWidth, g_winHeight);
    g_cam.drawCoordinate();

	// display the text
	drawText();

	g_mesh.draw(g_cam.viewMat, g_cam.projMat, 
		{ g_lightPos, g_lightPosAlt, g_lightPosAlt2, g_lightPosAlt3, g_lightPosAlt4, g_lightPosAlt5 }, /*list of light position vectors*/
		(g_cam.eye - g_cam.lookat),
		(float)total_time.count(), high_freq);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
	g_winWidth = w;
	g_winHeight = h;
	if (h == 0) {
		h = 1;
	}
	g_cam.setProjectionMatrix(g_winWidth, g_winHeight);
    g_cam.setViewMatrix();
    glViewport(0, 0, w, h);
}

void mouse(int button, int state, int x, int y)
{
    g_cam.mouseClick(button, state, x, y, g_winWidth, g_winHeight);
	//g_cam.mouseMotion(x, y, g_winWidth, g_winHeight);
}

void changeOrbitSpeed(int option)
{
	switch (option)
	{
	default:
		return;
	case sloworbit:
		orbitSpeedDenom = ORBIT_SPD_SLOW;
		break;
	case medorbit:
		orbitSpeedDenom = ORBIT_SPD_MED;
		break;
	case fastorbit:
		orbitSpeedDenom = ORBIT_SPD_FAST;
		break;
	case ultraorbit:
		orbitSpeedDenom = ORBIT_SPD_ULTRA;
		break;
	case stoporbit:
		orbitSpeedDenom = 0;
		break;
	}
}

void menu(int value)
{
	if (value < ah.song_list.size()) //the value will only be less than the song list size if a song is to be played
	{
		songT = 0;
		song_time_period = STP;
		song_ending = false;
		ah.play(value);
	}
	else
	{
		int selection = value - ah.song_list.size(); //pare down value so I don't have to go back through and rescale all the hard-coded options
		switch (selection)
		{
		default:
			break;
		case 0:
			g_cam.set(3.0f, 4.0f, 14.0f, 0.0f, 1.0f, -0.5f, g_winWidth, g_winHeight);
			break;
		case 1:
			g_cam.ChangeScrollSpeed(slowscroll);
			break;
		case 2:
			g_cam.ChangeScrollSpeed(medscroll);
			break;
		case 3:
			g_cam.ChangeScrollSpeed(fastscroll);
			break;
		case 4:
			changeOrbitSpeed(sloworbit);
			break;
		case 5:
			changeOrbitSpeed(medorbit);
			break;
		case 6:
			changeOrbitSpeed(fastorbit);
			break;
		case 7:
			changeOrbitSpeed(ultraorbit);
			break;
		case 8:
			changeOrbitSpeed(stoporbit);
			break;
		case 9: //vvvvv currently dummied out vvvvv
			g_mesh.changeMeshNoiseLevel(nonoise);
			break;
		case 10:
			g_mesh.changeMeshNoiseLevel(lownoise);
			break;
		case 11:
			g_mesh.changeMeshNoiseLevel(mednoise);
			break;
		case 12:
			g_mesh.changeMeshNoiseLevel(highnoise);
			break;//^^^^^ currently dummied out ^^^^^
		case 13:
			ah.stop();
			songT = 0;
			song_time_period = 0;
			orbitrad = MIN_ORBIT_RAD;
			high_freq = 0;
			low_freq = 0;
			break;
		case 14:
			text_display_select = tut1;
			break;
		case 15:
			text_display_select = tut2;
			break;
		case 16:
			text_display_select = lights;
			break;
		case 17:
			text_display_select = mesh;
			break;

				//....

		}
	}
	glutPostRedisplay();
}

void createMenu()
{
	int song_index_offset; //offset for all other (hard-coded) menus

	int songSelectMenu = glutCreateMenu(menu);

	//create menu entries for songs dynamically (on start-up) based on the song list
	for (song_index_offset = 0; song_index_offset < ah.song_list.size(); song_index_offset++)
	{
		glutAddMenuEntry(ah.song_list[song_index_offset].c_str(), song_index_offset);
	}
	//song_index_offset can now be added to future menu items to guarantee separation from song indices
	//in practice, this should always end up being equal to the song_list size

	int scrollMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Slow", 1 + song_index_offset);
	glutAddMenuEntry("Medium", 2 + song_index_offset);
	glutAddMenuEntry("Fast", 3 + song_index_offset);

	int orbitMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Stop Orbit", 8 + song_index_offset);
	glutAddMenuEntry("Slow", 4 + song_index_offset);
	glutAddMenuEntry("Medium", 5 + song_index_offset);
	glutAddMenuEntry("Fast", 6 + song_index_offset);
	glutAddMenuEntry("Ultra", 7 + song_index_offset);

	int textMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Controls", 14 + song_index_offset);
	glutAddMenuEntry("Mechanical Explanation", 15 + song_index_offset);
	glutAddMenuEntry("Light Stats", 16 + song_index_offset);
	glutAddMenuEntry("Mesh Data", 17 + song_index_offset);

	//int meshNoiseMenu = glutCreateMenu(menu);
	//glutAddMenuEntry("None", 9 + song_index_offset);
	//glutAddMenuEntry("Low", 10 + song_index_offset);
	//glutAddMenuEntry("Med", 11 + song_index_offset);
	//glutAddMenuEntry("High", 12 + song_index_offset);

	//....

	glutCreateMenu(menu);
	glutAddMenuEntry("Reset Camera", 0 + song_index_offset);
	glutAddSubMenu("Text Widget Display", textMenu);
	glutAddSubMenu("Scroll Zoom Speed", scrollMenu);
	glutAddSubMenu("Light Orbit Speed", orbitMenu);
	//glutAddSubMenu("Mesh Noise Level", meshNoiseMenu);
	glutAddSubMenu("Song Selection", songSelectMenu);
	glutAddMenuEntry("Stop Playback", 13 + song_index_offset);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

void motion(int x, int y)
{
    g_cam.mouseMotion(x, y, g_winWidth, g_winHeight);
}

void keyup(unsigned char key, int x, int y)
{
    g_keyStates[key] = false;
}

void keyboard(unsigned char key, int x, int y)
{
    g_keyStates[key] = true;
	switch(key) { 
		case 27:
			exit(0);
			break;
		case 9:
			swap_lights = (swap_lights + 1)%6;
			break;
        case'c': // switch cam control mode
            g_cam.switchCamMode();
			glutPostRedisplay();
            break;
        case' ':
            g_cam.PrintProperty();
            break;
		case'-':
			if (d_threshold > 1) d_threshold--;
			break;
		case'=':
			if (d_threshold < 17) d_threshold++;
			break;
		case'[':
			if (d_boost > 1) d_boost--;
			break;
		case']':
			if (d_boost < 20) d_boost++;
			break;
		case'r':
			switch(swap_lights) 
			{
			case 0:
				g_lightPos.y += 0.5f;
				break;
			case 1:
				g_lightPosAlt.y += 0.5f;
				break;
			case 2:
				g_lightPosAlt2.y += 0.5f;
				break;
			case 3:
				g_lightPosAlt3.y += 0.5f;
				break;
			case 4:
				g_lightPosAlt4.y += 0.5f;
				break;
			case 5:
				g_lightPosAlt5.y += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'f':
			switch (swap_lights)
			{
			case 0:
				g_lightPos.y -= 0.5f;
				break;
			case 1:
				g_lightPosAlt.y -= 0.5f;
				break;
			case 2:
				g_lightPosAlt2.y -= 0.5f;
				break;
			case 3:
				g_lightPosAlt3.y -= 0.5f;
				break;
			case 4:
				g_lightPosAlt4.y -= 0.5f;
				break;
			case 5:
				g_lightPosAlt5.y -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case'd':
			switch (swap_lights)
			{
			case 0:
				g_lightPos.x += 0.5f;
				break;
			case 1:
				g_lightPosAlt.x += 0.5f;
				break;
			case 2:
				g_lightPosAlt2.x += 0.5f;
				break;
			case 3:
				g_lightPosAlt3.x += 0.5f;
				break;
			case 4:
				g_lightPosAlt4.x += 0.5f;
				break;
			case 5:
				g_lightPosAlt5.x += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'a':
			switch (swap_lights)
			{
			case 0:
				g_lightPos.x -= 0.5f;
				break;
			case 1:
				g_lightPosAlt.x -= 0.5f;
				break;
			case 2:
				g_lightPosAlt2.x -= 0.5f;
				break;
			case 3:
				g_lightPosAlt3.x -= 0.5f;
				break;
			case 4:
				g_lightPosAlt4.x -= 0.5f;
				break;
			case 5:
				g_lightPosAlt5.x -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case's':
			switch (swap_lights)
			{
			case 0:
				g_lightPos.z += 0.5f;
				break;
			case 1:
				g_lightPosAlt.z += 0.5f;
				break;
			case 2:
				g_lightPosAlt2.z += 0.5f;
				break;
			case 3:
				g_lightPosAlt3.z += 0.5f;
				break;
			case 4:
				g_lightPosAlt4.z += 0.5f;
				break;
			case 5:
				g_lightPosAlt5.z += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'w':
			switch (swap_lights)
			{
			case 0:
				g_lightPos.z -= 0.5f;
				break;
			case 1:
				g_lightPosAlt.z -= 0.5f;
				break;
			case 2:
				g_lightPosAlt2.z -= 0.5f;
				break;
			case 3:
				g_lightPosAlt3.z -= 0.5f;
				break;
			case 4:
				g_lightPosAlt4.z -= 0.5f;
				break;
			case 5:
				g_lightPosAlt5.z -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case'q': //reset light positions and angles
			thetaAlt5 = THETAALT5_0;
			thetaAlt4 = THETAALT4_0;
			thetaAlt3 = THETAALT3_0;
			thetaAlt2 = THETAALT2_0;
			thetaAlt = THETAALT_0;
			theta = THETA_0;

			g_lightPosAlt5.x = orbitrad * std::cos(thetaAlt5);
			g_lightPosAlt4.x = 0;
			g_lightPosAlt3.x = orbitrad * std::cos(thetaAlt3);
			g_lightPosAlt2.x = orbitrad * std::cos(thetaAlt2);
			g_lightPosAlt.x = 0;
			g_lightPos.x = orbitrad * std::cos(theta);

			g_lightPosAlt5.y = orbitrad * std::sin(thetaAlt5);
			g_lightPosAlt4.y = orbitrad * std::sin(thetaAlt4);
			g_lightPosAlt3.y = 0;
			g_lightPosAlt2.y = orbitrad * std::sin(thetaAlt2);
			g_lightPosAlt.y = orbitrad * std::sin(thetaAlt);
			g_lightPos.y = 0;

			g_lightPosAlt5.z = 0;
			g_lightPosAlt4.z = orbitrad * std::cos(thetaAlt4);
			g_lightPosAlt3.z = orbitrad * std::sin(thetaAlt3);
			g_lightPosAlt2.z = 0;
			g_lightPosAlt.z = orbitrad * std::cos(thetaAlt);
			g_lightPos.z = orbitrad * std::sin(theta);
			break;
	}
	glutPostRedisplay();
}

/*void arrowkey(int key, int x, int y)
{
	switch(key)
	{
	case GLUT_KEY_UP:
		g_lightPos.y += 0.5;
		break;
	case GLUT_KEY_DOWN:
		g_lightPos.y -= 0.5;
		break;
	case GLUT_KEY_RIGHT:
		g_lightPos.x += 0.5;
		break;
	case GLUT_KEY_LEFT:
		g_lightPos.x -= 0.5;
		break;
	}
}*/

int main(int argc, char **argv) 
{
	glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(g_winWidth, g_winHeight);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("QPoyneer's Per-Vertex Musical Shades");
	
	glewInit();
	initialGL();

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
    glutKeyboardUpFunc(keyup);
    glutKeyboardFunc(keyboard);

	//glutSpecialFunc(arrowkey);

	glutIdleFunc(idle);

	initialization();

	createMenu();
	
    glutMainLoop();
    return EXIT_SUCCESS;
}