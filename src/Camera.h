#pragma once
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/constants.hpp>

using namespace glm;

//Original code was written by Prof. Chao Peng for IGME740 Assignment 3 project example,
//modified by Quinn Poyneer initially for Assignment 3 submission and
//subsequently for the Final Project of both IGME670 and IGME740

class Camera
{
public:
    vec4			eye;
    vec4			lookat;

	mat4			viewMat;
    mat4			projMat;

private:
    float           fovy;              // view angle 
    float           aspect;            // the window ration equal to w/h
    float           near_plane;
    float           far_plane;

    vec4			axis_n;            //eye-lookat
    vec4		    axis_u;
    vec4	        axis_v;            //up axis
    vec4			world_up;

    vec2			mouse_pos;         // the cursor position of moving mouse
    vec2			mouse_pre_pos;
    unsigned short  mouse_button;

    vec2			key_pos;           // the translated step changed by keyboard operation in FP cam mode
    vec2			key_pre_pos;
    bool            m_altKey;          // detect if ALT pressed 

    
    int m_mode;
	
    // View Frustum
	vec4	     ntl;	          // the near-top-left
	vec4         ntr;	          // the near-top-right
	vec4         nbl;	          // the near-bottom-left
	vec4         nbr;	          // the near-bottom-right
	vec4	     ftl;	          // the far-top-left
	vec4	     fbr;	          // the far-bottom-right
	vec4         ftr;	          // the far-top-right
	vec4	     fbl;	          // the far-bottom-left

    // scale parameters for the camera control
	float M_ZOOM_PAR;
	float M_PAN_PAR;
	float M_PAN_PAR_FP;
	float M_ROTATE_PAR_FP;

public: 
	Camera();
	~Camera();
	void PrintProperty();
	void set(float eye_x, float eye_y, float eye_z, 
			 float lookat_x, float lookat_y, float lookat_z, 
             int winW, int winH,
			 float p_angle = 45.0f, float p_near =0.1f, float p_far=10000.0f);

    void setProjectionMatrix(int winW, int winH);
    void setViewMatrix();
    void mouseClick(int button, int state, int x, int y, int winW ,int winH);
    void mouseMotion(int x, int y, int winW, int winH);
    void keyOperation(const unsigned char keyStates[], int winW, int winH);

	// cam mode
	void switchCamMode();
	bool isFocusMode();
	bool isFPMode();

	// focus cam mode 
	void CameraRotate();
	//void CameraZoom(); //dummied out to make room for scroll-wheel zoom ~QP
	void CameraPan();

	// first-person cam mode
	void CameraPan_fp();
	void CameraRotate_fp(int winW, int winH);

    void drawGrid();
    void drawCoordinate();
    void drawCoordinateOnScreen(int winW, int winH);

	void drawFrustum();

	//custom bs by QP
	void ChangeScrollSpeed(int option);

private: 
    void horizontalRotate();    // for focus cam mode
	void verticalRotate();      // for focus cam mode
    void GetCamCS();
	void GetViewFrustum();
};