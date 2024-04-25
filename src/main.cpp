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

const float THETA_0 = pi<float>() / 4.0f;
float theta = THETA_0;

const float THETAALT_0 = 3 * pi<float>() / 4.0f;
float thetaAlt = THETAALT_0;

float deltaT;
float oldT = 0;

const float ORBIT_RAD = 7.0f;

//divisor for 
const float ORBIT_SPD_SLOW = 1200;
const float ORBIT_SPD_MED = 600;
const float ORBIT_SPD_FAST = 300;
const float ORBIT_SPD_ULTRA = 150;

float orbitSpeedDenom;

vec3 g_lightPos = vec3(ORBIT_RAD * std::cos(theta), 0, ORBIT_RAD * std::sin(theta));
vec3 g_lightPosAlt = vec3(0, ORBIT_RAD * std::sin(thetaAlt), ORBIT_RAD * std::cos(thetaAlt));

bool swap_lights = false;

AudioHandler ah; //string audio_file;

float g_time = 0.0f;

void initialization() 
{    
    g_cam.set(3.0f, 4.0f, 14.0f, 0.0f, 1.0f, -0.5f, g_winWidth, g_winHeight);
	g_text.setColor(0.0f, 0.0f, 0.0f);

	deltaT = clock() - oldT;
	oldT = clock();
	
	orbitSpeedDenom = ORBIT_SPD_MED;

	ah.create(
		{
			"sounds\\Rabi-Ribi Original Soundtrack - 45 No Remorse.wav",
			"sounds\\zx_bgm024.wav",
			"sounds\\Necromantic.wav"
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

	//settin up light sources to orbit the origin
	//could've probably used gl rotate methods for this, but this was more fun and intuitive for me
	deltaT = clock() - oldT;
	oldT = clock();
	if (orbitSpeedDenom > 0)
	{
		theta += ((pi<float>() * deltaT) / orbitSpeedDenom);
		thetaAlt += ((pi<float>() * deltaT) / orbitSpeedDenom);

		if (theta >= (2 * pi<float>())) theta -= (2 * pi<float>());
		if (thetaAlt >= (2 * pi<float>())) thetaAlt -= (2 * pi<float>());

		g_lightPos.x = ORBIT_RAD * std::cos(theta);
		g_lightPos.z = ORBIT_RAD * std::sin(theta);
		//g_lightPos.x += -1 * std::sin(theta);
		//g_lightPos.z += std::cos(theta);

		g_lightPosAlt.y = ORBIT_RAD * std::sin(thetaAlt);
		g_lightPosAlt.z = ORBIT_RAD * std::cos(thetaAlt);
	}

	// add any stuff you'd like to draw	

	glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(value_ptr(g_cam.viewMat));
	glPushMatrix();

	glColor3f(1, 1, 0);
	
	glTranslatef(g_lightPos.x, g_lightPos.y, g_lightPos.z);
	(!swap_lights ? glutSolidSphere : glutWireSphere)(0.3, 8, 8);

	glPopMatrix();
	glColor3f(1, 0, 1);

	glTranslatef(g_lightPosAlt.x, g_lightPosAlt.y, g_lightPosAlt.z);
	(!swap_lights ? glutWireSphere : glutSolidSphere)(0.3, 8, 8);

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

	//str = "e";
	//g_text.draw(10, 75, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	//
	//str = "f";
	//g_text.draw(10, 90, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	str = "light position: (" + std::to_string(g_lightPos.x) + ", " 
		+ std::to_string(g_lightPos.y) + ", "
		+ std::to_string(g_lightPos.z) + ")";
	g_text.draw(10, 75, const_cast<char*>(str.c_str()), g_winWidth, g_winHeight);
	
	str = "2nd light position: (" + std::to_string(g_lightPosAlt.x) + ", "
		+ std::to_string(g_lightPosAlt.y) + ", "
		+ std::to_string(g_lightPosAlt.z) + ")";
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

	g_time = (float)glutGet(GLUT_ELAPSED_TIME)/1000.0f;
	//g_mesh.draw(g_cam.viewMat, g_cam.projMat, g_lightPos, g_time);

	g_mesh.drawAlt(g_cam.viewMat, g_cam.projMat, g_lightPos, g_lightPosAlt, vec3(g_cam.lookat.x, g_cam.lookat.y, g_cam.lookat.z), g_time);
	//g_mesh2.drawAlt(g_cam.viewMat, g_cam.projMat, g_lightPos, g_lightPosAlt, vec3(g_cam.lookat.x, g_cam.lookat.y, g_cam.lookat.z), g_time);

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
		cout << "fuck me sideways" << endl;
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
		break;
	case 14:
		ah.play(0);
		break;
	case 15:
		ah.play(1);
		break;
	case 16:
		ah.play(2);
		
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
			swap_lights = !swap_lights;
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
		case'+':
			g_mesh.normal_offset -= 0.01;
			//glutPostRedisplay();
			break;
		case'r':
			(swap_lights ? g_lightPosAlt.y : g_lightPos.y) += 0.5;
			break;
		case'f':
			(swap_lights ? g_lightPosAlt.y : g_lightPos.y) -= 0.5;
			break;
		case'd':
			(swap_lights ? g_lightPosAlt.x : g_lightPos.x) += 0.5;
			break;
		case'a':
			(swap_lights ? g_lightPosAlt.x : g_lightPos.x) -= 0.5;
			break;
		case's':
			(swap_lights ? g_lightPosAlt.z : g_lightPos.z) += 0.5;
			break;
		case'w':
			(swap_lights ? g_lightPosAlt.z : g_lightPos.z) -= 0.5;
			break;
		case'q':
			(swap_lights ? 
				thetaAlt = THETAALT_0 : 
				theta = THETA_0);
			(swap_lights ? 
				g_lightPosAlt.x = 0 : 
				g_lightPos.x = ORBIT_RAD * std::cos(theta));
			(swap_lights ? 
				g_lightPosAlt.y = ORBIT_RAD * std::sin(thetaAlt) : 
				g_lightPos.y = 0);
			(swap_lights ? 
				g_lightPosAlt.z = ORBIT_RAD * std::cos(thetaAlt) :
				g_lightPos.z = ORBIT_RAD * std::sin(theta));
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