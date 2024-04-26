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
//#define _USES_MATH_DEFINES //pulled from this source: https://stackoverflow.com/questions/1727881/how-to-use-the-pi-constant-in-c
//#include <math.h>
using namespace std;
using namespace glm;

// Original code was written by Prof. Chao Peng for IGME740 Assignment 3 project example,
// modified by Quinn Poyneer initially for Assignment 3 submission and
// subsequently for the Final Project of both IGME670 and IGME740
// 
// Uses libsndfile for audio data handling: https://github.com/libsndfile/libsndfile
// Tutorial for basic use found here: https://cindybui.me/pages/blogs/visual_studio_0
// 

int g_winWidth  = 1024;
int g_winHeight = 512;

Camera g_cam;
Text g_text;

enum ScrollSpeed { slowscroll, medscroll, fastscroll };
enum OrbitSpeed { sloworbit, medorbit, fastorbit, ultraorbit, stoporbit };
enum NoiseLevels { nonoise, lownoise, mednoise, highnoise };

unsigned char g_keyStates[256];

char v_shader_file[] =
//".\\shaders\\basic.vert";
//".\\shaders\\displacement.vert";		// vertex displacement shader with perlin noise
//".\\shaders\\perVert_lambert.vert";	// basic lambert lighting  
//".\\shaders\\perVert_phong.vert";		// the big boy PHONG
//".\\shaders\\perFrag_lambert.vert";	// basic lambert lighting with per-fragment implementation
//".\\shaders\\perFrag_phong.vert";		// the big boy PHONG (per frag)
//".\\shaders\\toon_shading.vert";		// basic toon shading with per-fragment implementation
// 
//
".\\shaders\\per_Vertex_CUSTOM.vert";	// CUSTOM phong + displacement for warping a sphere in time with music


char f_shader_file[] =
//".\\shaders\\basic.frag";
//".\\shaders\\displacement.frag";		// vertex displacement shader with perlin noise
//".\\shaders\\perVert_lambert.frag";	// basic lambert shading 
//".\\shaders\\perVert_phong.frag";		// the big boy PHONG
//".\\shaders\\perFrag_lambert.frag";	// basic lambert shading with per-fragment implementation
//".\\shaders\\perFrag_phong.frag";		// the big boy PHONG (per frag)
//".\\shaders\\toon_shading.frag";		// basic toon shading with per-fragment implementation
//
// 
".\\shaders\\per_Vertex_CUSTOM.frag";	// CUSTOM phong + displacement for warping a sphere in time with music


//char v_shader_file2[] =
//".\\shaders\\basic.vert";
//".\\shaders\\displacement.vert";		// vertex displacement shader with perlin noise
// 
//".\\shaders\\perVert_lambert.vert";	// basic lambert lighting
//".\\shaders\\perVert_phong.vert";		// the big boy PHONG
//
//".\\shaders\\perFrag_lambert.vert";	// basic lambert lighting with per-fragment implementation
//".\\shaders\\perFrag_phong.vert";		// the big boy PHONG (per frag)
// 
//".\\shaders\\toon_shading.vert";		// basic toon shading with per-fragment implementation
//
//char f_shader_file2[] =
//".\\shaders\\basic.frag";
//".\\shaders\\displacement.frag";		// vertex displacement shader with perlin noise
//".\\shaders\\perVert_lambert.frag";	// basic lambert shading
//".\\shaders\\perVert_phong.frag";		// the big boy PHONG
//
//".\\shaders\\perFrag_lambert.frag";	// basic lambert shading with per-fragment implementation
//".\\shaders\\perFrag_phong.frag";		// the big boy PHONG (per frag)
// 
//".\\shaders\\toon_shading.frag";		// basic toon shading with per-fragment implementation

const char meshFile[128] =
"Mesh/sphere.obj";
//"Mesh/bunny2K.obj";
//"Mesh/teapot.obj";
//"Mesh/teddy.obj";

//const char meshFile2[128] =
//"Mesh/sphere.obj";
//"Mesh/bunny2K.obj";
//"Mesh/teapot.obj";
//"Mesh/teddy.obj";

Mesh g_mesh;
//Mesh g_mesh2;

wstring beef;

const float THETA_0 = pi<float>() / 4.0f; //about y
float theta = THETA_0;

const float THETAALT_0 = 3 * pi<float>() / 4.0f; //about x
float thetaAlt = THETAALT_0;

const float THETAALT2_0 = pi<float>() / 2.0f; //about z
float thetaAlt2 = THETAALT2_0;

const float THETAALT3_0 = 5 * pi<float>() / 4.0f; //about y (#2)
float thetaAlt3 = THETAALT3_0;

const float THETAALT4_0 = 7 * pi<float>() / 4.0f; //about x (#2)
float thetaAlt4 = THETAALT4_0;

const float THETAALT5_0 = 3 * pi<float>() / 2.0f; //about z (#2)
float thetaAlt5 = THETAALT5_0;

float deltaT;
float oldT = 0;

//base orbit radius
const float MIN_ORBIT_RAD = 7.0f;
float orbitrad = MIN_ORBIT_RAD;

//divisors for orbit speed -- higher number = slower orbit
const float ORBIT_SPD_SLOW = 1.200;
const float ORBIT_SPD_MED = 0.600;
const float ORBIT_SPD_FAST = 0.300;
const float ORBIT_SPD_ULTRA = 0.150;

float orbitSpeedDenom;

//orbiting y
vec3 g_lightPos = vec3(
	orbitrad * std::cos(theta), 
	0.0f, 
	orbitrad * std::sin(theta));

vec3 g_lightPosAlt3 = vec3(
	orbitrad * std::cos(thetaAlt3), 
	0.0f, 
	orbitrad * std::sin(thetaAlt3));

//orbiting x
vec3 g_lightPosAlt = vec3(
	0.0f, 
	orbitrad * std::sin(thetaAlt), 
	orbitrad * std::cos(thetaAlt));

vec3 g_lightPosAlt4 = vec3(
	0.0f, 
	orbitrad * std::sin(thetaAlt4), 
	orbitrad * std::cos(thetaAlt4));

//orbiting z
vec3 g_lightPosAlt2 = vec3(
	orbitrad * std::cos(thetaAlt2), 
	orbitrad * std::sin(thetaAlt2), 
	0.0f);

vec3 g_lightPosAlt5 = vec3(
	orbitrad * std::cos(thetaAlt5), 
	orbitrad * std::sin(thetaAlt5), 
	0.0f);

int swap_lights = 0;

AudioHandler ah;	//audio handler -- holds song list, plays music, and parses song data
//int song_frame = 0;
float low_freq;		//average low-frequency amplitude
float high_freq;	//average high-frequency amplitude
float song_time = 0;	//time since the song began
bool song_ending;	//if the song will end within the next few ms

float song_time_period = 0;	//tracks period for extracting fft data
const float STP = 0.04;

float g_time = 0.0f;

void initialization() 
{    
    g_cam.set(3.0f, 4.0f, 14.0f, 0.0f, 1.0f, -0.5f, g_winWidth, g_winHeight);
	g_text.setColor(0.0f, 0.0f, 0.0f);
	
	orbitSpeedDenom = ORBIT_SPD_MED; //default to medium orbit speed

	ah.create(
		{
			"sounds\\Rabi-Ribi Original Soundtrack - 45 No Remorse.wav",
			"sounds\\zx_bgm024.wav",
			"sounds\\Necromantic.wav",
			"sounds\\rrgo.wav"
		});

	mat4 m = translate(mat4(1.0), vec3(0.0f, 0.0f, 0.0f));
	m = rotate(m, 3.14159f, vec3(0.0f, 1.0f, 0.0f));
	g_mesh.modelMat = scale(m, vec3(0.5f, 0.5f, 0.5f));
	g_mesh.create(meshFile, v_shader_file, f_shader_file);

	//mat4 m2 = translate(mat4(1.0), vec3(3.0f, 2.0f, 0.0f));
	//g_mesh2.modelMat = scale(m2, vec3(0.5f, 0.5f, 0.5f));
	//g_mesh2.create(meshFile, v_shader_file2, f_shader_file2);

	// add any stuff you want to initialize ...

	//audio_file =
	//	//"zx_bgm024.wav";
	//	"Rabi-Ribi Original Soundtrack - 45 No Remorse.wav";
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

void display()
{	 
	glClearColor(1.0, 1.0, 1.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	mat4 mvp = g_cam.projMat * g_cam.viewMat;
	g_time = (float)glutGet(GLUT_ELAPSED_TIME) / 1000.0f; //moved this up here for ~reasons~
	//cout << std::to_string(deltaT) << endl;

	//settin up light sources to orbit the origin
	//could've probably used gl rotate methods for this, but this was more fun and intuitive for me
	deltaT = g_time - oldT;
	oldT = g_time;

	//orbitrad += low_freq;
	orbitrad = MIN_ORBIT_RAD * (1 + (low_freq/* * (10 * (-1 * powf(song_time_period - std::sqrtf(.05), 2) + .05))*/));

	if (orbitSpeedDenom > 0)
	{
		theta += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt2 += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt3 += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt4 += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt5 += ((pi<float>() * deltaT) / orbitSpeedDenom);
	}

	if (theta >= (2 * pi<float>())) theta -= (2 * pi<float>());
	if (thetaAlt >= (2 * pi<float>())) thetaAlt -= (2 * pi<float>());

	g_lightPos.x = orbitrad * std::cos(theta);
	g_lightPos.z = orbitrad * std::sin(theta);
	//g_lightPos.x += -1 * std::sin(theta);
	//g_lightPos.z += std::cos(theta);

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

	//orbitrad = std::max(orbitrad - low_freq, MIN_ORBIT_RAD);

	if (ah.is_playing && !song_ending) //if the song is playing and isn't about to end, 
	{
		song_time_period += deltaT;
		song_time += deltaT;
		if (song_time_period > STP)
		{
			low_freq = 0;
			high_freq = 0;
			//song will end within approximately ten draw calls?
			song_ending = ah.extractfft(song_time, song_time_period, low_freq, high_freq);
			//cout << "Low freq avg m = " + std::to_string(low_freq) +
			//	"; High freq avg m = " + std::to_string(high_freq) << endl;

			song_time_period = 0;
		}
	}
	else if(song_time > 0)
	{
		cout << "Song ended at time: " + std::to_string(song_time) + " / " + std::to_string(ah.duration) << endl;
		song_time = 0;
		//song_ending = false;
		song_time_period = 0;//STP;
	}

	// add any stuff you'd like to draw	

	glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(value_ptr(g_cam.viewMat));
	glPushMatrix();
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
	glColor3f(1, 0, 0); //set up light 3 (red, orbit z)
	glTranslatef(g_lightPosAlt5.x, g_lightPosAlt5.y, g_lightPosAlt5.z);
	(swap_lights == 5 ? glutSolidSphere : glutWireSphere)(0.2, 8, 8);

	glPopMatrix();

	g_cam.drawGrid();
    g_cam.drawCoordinateOnScreen(g_winWidth, g_winHeight);
    g_cam.drawCoordinate();

	// display the text
	string str;
	if(g_cam.isFocusMode()) {
        str = "Cam mode: Focus\n";
		g_text.draw(10, 30, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	} else if(g_cam.isFPMode()) {
		str = "Cam mode: FP";
		g_text.draw(10, 30, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	}
	str = "b";
	g_text.draw(10, 45, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	str = "e";
	g_text.draw(10, 60, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	str = "FPS: " + std::to_string(1000/deltaT);
	g_text.draw(10, 75, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	
	str = "Frame Time: " + std::to_string(deltaT);
	g_text.draw(10, 90, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	str = "Theta value: " + std::to_string(theta);
	g_text.draw(10, 105, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	str = "ThetaAlt value: " + std::to_string(thetaAlt);
	g_text.draw(10, 120, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	str = "Current BGM: " + (ah.is_playing ? ah.song_list[ah.now_playing] : "N/A");
	g_text.draw(10, 135, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);


	//str = "vertex count: " + std::to_string(g_mesh.vert_num);
	//g_text.draw(10, 45, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	//str = "triangle count: " + std::to_string(g_mesh.tri_num);
	//g_text.draw(10, 60, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	//
	//str = "light position: (" + std::to_string(g_lightPos.x) + ", " 
	//	+ std::to_string(g_lightPos.y) + ", "
	//	+ std::to_string(g_lightPos.z) + ")";
	//g_text.draw(10, 75, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	//
	//str = "2nd light position: (" + std::to_string(g_lightPosAlt.x) + ", "
	//	+ std::to_string(g_lightPosAlt.y) + ", "
	//	+ std::to_string(g_lightPosAlt.z) + ")";
	//g_text.draw(10, 90, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);

	//g_mesh.draw(g_cam.viewMat, g_cam.projMat, g_lightPos, g_time);

	g_mesh.draw(g_cam.viewMat, g_cam.projMat, 
		{ g_lightPos, g_lightPosAlt, g_lightPosAlt2, g_lightPosAlt3, g_lightPosAlt4, g_lightPosAlt5 }, /*list of light position vectors*/
		vec3(g_cam.lookat.x, g_cam.lookat.y, g_cam.lookat.z), 
		g_time);
	//g_mesh2.draw(g_cam.viewMat, g_cam.projMat, g_lightPos, g_lightPosAlt, vec3(g_cam.lookat.x, g_cam.lookat.y, g_cam.lookat.z), g_time);

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
	switch (value)
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
	case 9:
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
		break;
	case 13:
		ah.stop();
		song_time = 0;
		song_time_period = 0;
		orbitrad = MIN_ORBIT_RAD;
		break;
	case 14:
		song_time_period = 0;// STP;
		song_ending = false;
		ah.play(0);
		break;
	case 15:
		song_time_period = 0;// STP;
		song_ending = false;
		ah.play(1);
		break;
	case 16:
		song_time_period = 0;// STP;
		song_ending = false;
		ah.play(2);
		break;
	case 17:
		song_time_period = 0;// STP;
		song_ending = false;
		ah.play(3);
		break;
		
		//....

	}
	glutPostRedisplay();
}

void createMenu()
{
	int scrollMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Slow", 1);
	glutAddMenuEntry("Medium", 2);
	glutAddMenuEntry("Fast", 3);

	int orbitMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Stop Orbit", 8);
	glutAddMenuEntry("Slow", 4);
	glutAddMenuEntry("Medium", 5);
	glutAddMenuEntry("Fast", 6);
	glutAddMenuEntry("Ultra", 7);

	int meshNoiseMenu = glutCreateMenu(menu);
	glutAddMenuEntry("None", 9);
	glutAddMenuEntry("Low", 10);
	glutAddMenuEntry("Med", 11);
	glutAddMenuEntry("High", 12);

	int songSelectMenu = glutCreateMenu(menu);
	glutAddMenuEntry("Rabi-Ribi Original Soundtrack - 45 No Remorse.wav", 14);
	glutAddMenuEntry("zx_bgm024.wav", 15);
	glutAddMenuEntry("Necromantic.wav", 16);
	glutAddMenuEntry("rrgo.wav", 17);

	//....

	glutCreateMenu(menu);
	glutAddMenuEntry("Reset Camera", 0);
	glutAddMenuEntry("Stop", 13);
	glutAddSubMenu("Scroll Zoom Speed", scrollMenu);
	glutAddSubMenu("Light Orbit Speed", orbitMenu);
	glutAddSubMenu("Mesh Noise Level", meshNoiseMenu);
	glutAddSubMenu("Song Selection", songSelectMenu);
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
			swap_lights = (swap_lights++)%6;
			break;
        case'c': // switch cam control mode
            g_cam.switchCamMode();
			glutPostRedisplay();
            break;
        case' ':
            g_cam.PrintProperty();
            break;
		case'-':
			g_mesh.normal_offset += 0.01;
			//glutPostRedisplay();
			break;
		case'=':
			g_mesh.normal_offset -= 0.01;
			//glutPostRedisplay();
			break;
		case'r':
			switch(swap_lights) 
			{
			case 1:
				g_lightPos.y += 0.5f;
				break;
			case 2:
				g_lightPosAlt.y += 0.5f;
				break;
			case 3:
				g_lightPosAlt2.y += 0.5f;
				break;
			case 4:
				g_lightPosAlt3.y += 0.5f;
				break;
			case 5:
				g_lightPosAlt4.y += 0.5f;
				break;
			case 6:
				g_lightPosAlt5.y += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'f':
			switch (swap_lights)
			{
			case 1:
				g_lightPos.y -= 0.5f;
				break;
			case 2:
				g_lightPosAlt.y -= 0.5f;
				break;
			case 3:
				g_lightPosAlt2.y -= 0.5f;
				break;
			case 4:
				g_lightPosAlt3.y -= 0.5f;
				break;
			case 5:
				g_lightPosAlt4.y -= 0.5f;
				break;
			case 6:
				g_lightPosAlt5.y -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case'd':
			switch (swap_lights)
			{
			case 1:
				g_lightPos.x += 0.5f;
				break;
			case 2:
				g_lightPosAlt.x += 0.5f;
				break;
			case 3:
				g_lightPosAlt2.x += 0.5f;
				break;
			case 4:
				g_lightPosAlt3.x += 0.5f;
				break;
			case 5:
				g_lightPosAlt4.x += 0.5f;
				break;
			case 6:
				g_lightPosAlt5.x += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'a':
			switch (swap_lights)
			{
			case 1:
				g_lightPos.x -= 0.5f;
				break;
			case 2:
				g_lightPosAlt.x -= 0.5f;
				break;
			case 3:
				g_lightPosAlt2.x -= 0.5f;
				break;
			case 4:
				g_lightPosAlt3.x -= 0.5f;
				break;
			case 5:
				g_lightPosAlt4.x -= 0.5f;
				break;
			case 6:
				g_lightPosAlt5.x -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case's':
			switch (swap_lights)
			{
			case 1:
				g_lightPos.z += 0.5f;
				break;
			case 2:
				g_lightPosAlt.z += 0.5f;
				break;
			case 3:
				g_lightPosAlt2.z += 0.5f;
				break;
			case 4:
				g_lightPosAlt3.z += 0.5f;
				break;
			case 5:
				g_lightPosAlt4.z += 0.5f;
				break;
			case 6:
				g_lightPosAlt5.z += 0.5f;
				break;
			default:
				break;
			}
			break;
		case'w':
			switch (swap_lights)
			{
			case 1:
				g_lightPos.z -= 0.5f;
				break;
			case 2:
				g_lightPosAlt.z -= 0.5f;
				break;
			case 3:
				g_lightPosAlt2.z -= 0.5f;
				break;
			case 4:
				g_lightPosAlt3.z -= 0.5f;
				break;
			case 5:
				g_lightPosAlt4.z -= 0.5f;
				break;
			case 6:
				g_lightPosAlt5.z -= 0.5f;
				break;
			default:
				break;
			}
			break;
		case'q':

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
    glutCreateWindow("QPoyneer's Musical Shades");
	
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

	createMenu();

	initialization();

	//wstring tomp = wstring(audio_file.begin(), audio_file.end());
	//
	//LPCWSTR beef = tomp.c_str();
	//
	//(PlaySoundW(beef,
	//	NULL, 
	//	SND_FILENAME /* | SND_NODEFAULT*/ | SND_ASYNC | SND_LOOP) ?
	//	cout << "success!" << endl : 
	//	cout << "failure..." << endl);
	
    glutMainLoop();
    return EXIT_SUCCESS;
}