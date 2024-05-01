#include <GL/glew.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif


#include "Camera.h"

#include <iostream>
using namespace std;

#define CAM_FOCUS		0
#define CAM_FP			1

//Original code was written by Prof. Chao Peng for IGME740 Assignment 3 project example,
//modified by Quinn Poyneer initially for Assignment 3 submission and
//subsequently for the Final Project of both IGME670 and IGME740

enum ScrollSpeed { slow, med, fast };

const float SPD_SLOW = 0.03;
const float SPD_MED = 0.09;
const float SPD_FAST = 0.18;

Camera:: Camera()
{
    eye = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    lookat = vec4(0.0f, 0.0f, 0.0f, 1.0f);
    
    axis_n = vec4(0.0f);
    axis_u = vec4(0.0f);
    axis_v = vec4(0.0f);

    fovy        = 0.0f;
    aspect      = 0.0f;
    near_plane  = 0.0f;
    far_plane   = 0.0f;

    world_up = vec4(0.0f, 1.0f, 0.0f, 0.0f);

	m_mode      = CAM_FOCUS;

    mouse_pos = vec2(0.0f);
    mouse_pre_pos = vec2(0.0f);
    mouse_button= 0;

    key_pos = vec2(0.0f);
    key_pre_pos = vec2(0.0f, 0.0f);
	m_altKey     = false;

	M_ZOOM_PAR   = SPD_SLOW;	//Made this changeable
	M_PAN_PAR    = 50.0f;
	M_PAN_PAR_FP = 0.03f;
	M_ROTATE_PAR_FP = 8.0f;
}

Camera :: ~Camera()
{
}

void Camera::PrintProperty()
{
	cout<<"********* Camera **********"<<endl;
    cout<<"eye: "<<eye.x<<", "<<eye.y<<", " <<eye.z<<endl;
    cout<<"lookat: "<<lookat.x <<", "<<lookat.y<<", "<<lookat.z<<endl;
	cout<<"angle = "<<fovy<<endl;
	cout<<"ratio = "<<aspect<<endl;
	cout<<"near = "<<near_plane<<endl;
	cout<<"far = "<<far_plane<<endl;
	cout<<"n = "<<axis_n.x <<", "<<axis_n.y<<", "<<axis_n.z<<endl;
	cout<<"u = "<<axis_u.x<<", "<<axis_u.y<<", "<<axis_u.z<<endl;
	cout<<"v = "<<axis_v.x<<", "<<axis_v.y<<", "<<axis_v.z<<endl;
	cout<<"~~~~~~~~~~~~~~~~~~~~~~~~~~~" << endl;
	//cout<<"fbl = "<<frustum.fbl.x<<", "<<frustum.fbl.y<<", "<<frustum.fbl.z<<endl;
	//cout<<"fbr = "<<frustum.fbr.x<<", "<<frustum.fbr.y<<", "<<frustum.fbr.z<<endl;
	//cout<<"ftl = "<<frustum.ftl.x<<", "<<frustum.ftl.y<<", "<<frustum.ftl.z<<endl;
	//cout<<"ftr = "<<frustum.ftr.x<<", "<<frustum.ftr.y<<", "<<frustum.ftr.z<<endl;

}

void Camera::set(float eye_x, float eye_y, float eye_z, 
				 float lookat_x, float lookat_y, float lookat_z,
				 int winW, int winH,
				 float p_angle, float p_near, float p_far)
{
	world_up.y = 1; //set/reset camera position *and world space up direction* ~QP
	eye = vec4(eye_x, eye_y, eye_z, 1.0f);
	lookat = vec4(lookat_x, lookat_y, lookat_z, 1.0f);
	
	fovy        = p_angle;
	near_plane  = p_near;
	far_plane   = p_far;

	setProjectionMatrix(winW, winH);
	setViewMatrix();
    GetCamCS();
    GetViewFrustum();
}

void Camera::switchCamMode() 
{
    m_mode = (m_mode == CAM_FOCUS) ? CAM_FP : CAM_FOCUS;
}

bool Camera::isFocusMode()
{
	if(m_mode==CAM_FOCUS) {
		return true;
	} else {
		return false;
	}
}

bool Camera::isFPMode()
{
	if(m_mode==CAM_FP) {
		return true;
	} else {
		return false;
	}
}

void Camera::setProjectionMatrix(int winW, int winH)
{
	aspect = (float)winW/(float)winH;
	projMat = perspective(radians(fovy), aspect, near_plane,far_plane);
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(value_ptr(projMat));
	
	//glLoadIdentity();
	//gluPerspective(fovy, aspect, near_plane, far_plane);
}

void Camera::setViewMatrix()
{	
	viewMat = lookAt(eye.xyz(), lookat.xyz(), world_up.xyz());
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(value_ptr(projMat));
	//glLoadIdentity();
	//gluLookAt(eye.x, eye.y, eye.z,
	//          lookat.x, lookat.y, lookat.z,
	//			world_up.x, world_up.y, world_up.z);
}

void Camera::mouseClick(int button, int state, int x, int y, int winW, int winH)
{
    if(x >= winW || y >= winH) {
        return;
    }
	mouse_button = (state == GLUT_DOWN) ? button : 0;
    
	if( m_mode == CAM_FP) {
		if(mouse_button == GLUT_LEFT_BUTTON) {
			mouse_pos = vec2((float)x, -(float)y);
			mouse_pre_pos = mouse_pos;
		}
	} else if(m_mode == CAM_FOCUS) {
        mouse_pos = vec2(x/(float)winW - 1.0f, (winH-y)/(float)winH - 1.0f);
		mouse_pre_pos = mouse_pos; 
        m_altKey = (glutGetModifiers() == GLUT_ACTIVE_ALT);
	}

	// Copied this segment from https://stackoverflow.com/questions/14378/using-the-mouse-scrollwheel-in-glu
	// Did this so the camera can be made to zoom with the scroll wheel (which I find much more intuitive)
	// Wheel reports as button 3(scroll up) and button 4(scroll down)
	if ((button == 3) || (button == 4)) // It's a wheel event
	{
		// Each wheel event reports like a button click, GLUT_DOWN then GLUT_UP
		if (state == GLUT_UP) return; // Disregard redundant GLUT_UP events
		//printf("Scroll %s At %d %d\n", (button == 3) ? "Up" : "Down", x, y);
		eye -= lookat;

		if (button == 4)
		{
			eye += eye * M_ZOOM_PAR;
			//lookat += eye * M_ZOOM_PAR; //this makes the lookat move along eye-lookat 
		}
		else {
			eye -= eye * M_ZOOM_PAR;
			//lookat -= eye * M_ZOOM_PAR; //this makes the lookat move along eye-lookat 
		}
		eye += lookat;
		setProjectionMatrix(winW, winH);	//copied these from the old zoom method
		setViewMatrix();
		GetCamCS();
		GetViewFrustum();
		glViewport(0, 0, winW, winH);
	}


	glutPostRedisplay();
}

void Camera::ChangeScrollSpeed(int option)
{
	switch (option)
	{
	default:
		return;
	case slow:
		M_ZOOM_PAR = SPD_SLOW;
		//cout << "Set Scroll to SLOW" << endl;
		break;
	case med:
		M_ZOOM_PAR = SPD_MED;
		//cout << "Set Scroll to MED" << endl;
		break;
	case fast:
		M_ZOOM_PAR = SPD_FAST;
		//cout << "Set Scroll to FAST" << endl;
		break;
	}
}

void Camera::mouseMotion(int x, int y, int winW, int winH)
{
    if(x >= winW || y >= winH) {
        return;
    }

	if( m_mode == CAM_FP){	
		if(mouse_button == GLUT_LEFT_BUTTON) {
			mouse_pos = vec2((float)x, -(float)y);
			CameraRotate_fp(winW, winH);
			mouse_pre_pos = mouse_pos;
		}
	} 
	else if( m_mode == CAM_FOCUS){
		mouse_pos = vec2(x/(float)winW - 1.0f, (winH-y)/(float)winH - 1.0f);
		if ((glutGetModifiers() == GLUT_ACTIVE_ALT)) //put this here so the menus would stop rotating the camera
		{
            if(mouse_button == GLUT_LEFT_BUTTON)
			{
                CameraRotate();
				//cout << "camera is rotato" << endl;
            }

			else if(mouse_button == GLUT_MIDDLE_BUTTON)
			{
                CameraPan();
            }
			
			//else if(mouse_button == GLUT_RIGHT_BUTTON)
			//{
            //    CameraZoom();
            //}
		}
		mouse_pre_pos = mouse_pos;
	}

	setProjectionMatrix(winW, winH);
	setViewMatrix();
	GetCamCS();
	GetViewFrustum();
    glViewport(0, 0, winW, winH);

	glutPostRedisplay();
}

void Camera::keyOperation(const unsigned char keyStates[], int winW, int winH)
{
    if(m_mode != CAM_FP) {
        return;
    }

    float speed = 1.0f;

    if(keyStates['g']) {
       	key_pos.x-=speed;
		CameraPan_fp();
		setProjectionMatrix(winW, winH);
		setViewMatrix();
		GetCamCS();
		GetViewFrustum();
        glViewport(0, 0, winW, winH);
		glutPostRedisplay();
        key_pre_pos.x = key_pos.x; 
    }
    if(keyStates['j']) {
       	key_pos.x+=speed;
		CameraPan_fp();
		setProjectionMatrix(winW, winH);
		setViewMatrix();
		GetCamCS();
		GetViewFrustum();
        glViewport(0, 0, winW, winH);
		glutPostRedisplay();
        key_pre_pos.x = key_pos.x; 
    }
    if(keyStates['y']) {
       	key_pos.y-=speed;
		CameraPan_fp();
		setProjectionMatrix(winW, winH);
		setViewMatrix();
		GetCamCS();
		GetViewFrustum();
        glViewport(0, 0, winW, winH);
		glutPostRedisplay();
        key_pre_pos.y = key_pos.y; 
    }
    if(keyStates['h']) {
       	key_pos.y+=speed;
		CameraPan_fp();
		setProjectionMatrix(winW, winH);
        setViewMatrix();
		GetCamCS();
		GetViewFrustum();
        glViewport(0, 0, winW, winH);
		glutPostRedisplay();
        key_pre_pos.y = key_pos.y; 
    }
}


void Camera::drawGrid()
{
    int size = 25;              // determing the grid size and the numbers of cells
    if(size%2 != 0) ++size;     // better to be an odd size

    glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(value_ptr(viewMat));

	//glScalef(30.0f, 30.0f, 30.0f);
	glBegin(GL_LINES);
	for (int i =0; i<size+1; i++) {
        if((float)i == size/2.0f) { 
            glColor3f(0.0f, 0.0f, 0.0f);
        } else {
            glColor3f(0.8f, 0.8f, 0.8f);
        }
		glVertex3f(-size/2.0f, 0.0f, -size/2.0f+i);
		glVertex3f(size/2.0f, 0.0f, -size/2.0f+i);
		glVertex3f(-size/2.0f+i, 0.0f, -size/2.0f);
		glVertex3f(-size/2.0f+i, 0.0f, size/2.0f);
	}
	glEnd();
	glPopMatrix();
}


void Camera::drawCoordinate()
{
    glUseProgram(0);
	glDisable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadMatrixf(value_ptr(viewMat));
	//glScalef(40.0f, 40.0f, 40.0f);
	
	glLineWidth(2.5f);
	glBegin(GL_LINES);

	//axis x	
	glColor3f(1.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.3f, 0.0f, 0.0f);

    //text x
	glVertex3f(0.4f, 0.05f, 0.0f);
	glVertex3f(0.5f, -0.05f, 0.0f);
	glVertex3f(0.4f, -0.05f, 0.0f);
	glVertex3f(0.5f, 0.05f, 0.0f);
	
	//axis y
	glColor3f(0.0f, 1.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.3f, 0.0f);
	
    //text y
    glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.0f, 0.4f, 0.0f);
	glVertex3f(-0.05f, 0.55f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	glVertex3f(0.05f, 0.55f, 0.0f);
	glVertex3f(0.0f, 0.5f, 0.0f);
	
	//axis z
	glColor3f(0.0f, 0.0f, 1.0f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 0.3f);
	
    //text z
    glVertex3f(-0.025f, 0.025f, 0.4f);
	glVertex3f(0.025f, 0.025f, 0.4f);
	glVertex3f(0.025f, 0.025f, 0.4f);
	glVertex3f(-0.025f, -0.025f, 0.4f);
	glVertex3f(-0.025f, -0.025f, 0.4f);
	glVertex3f(0.025f, -0.025f, 0.4f);
	
    glEnd();
	glLineWidth(1.0f);

	glPopMatrix();
}

void Camera::drawCoordinateOnScreen(int winW, int winH)
{
    vec4 ax(1.0f, 0.0f, 0.0f, 0.0f);
    vec4 ay(0.0f, 1.0f, 0.0f, 0.0f);
    vec4 az(0.0f, 0.0f, 1.0f, 0.0f);

    mat4 mvp = projMat*viewMat;
    vec4 sx = mvp*ax;
    vec4 sy = mvp*ay;
    vec4 sz = mvp*az;
    float ratio = (float)winW/(float)winH;

    glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
	gluOrtho2D(-1.0f, 1.0f, -1.0f, 1.0f);
    
    glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
    glLoadIdentity();
    glTranslatef(-0.92f, -0.85f, 0.0f);   // position of the coordinate
    glScalef(0.03f, 0.03f, 0.03f);        // size of the coorinate
	//glScalef(0.3f, 0.3f, 0.3f);
    glLineWidth(2.0f);

    // axis x
    glBegin(GL_LINES);
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(sx.x, sx.y);
    //axis y
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(sy.x, sy.y);
    //axis z
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex2f(0.0f, 0.0f);
    glVertex2f(sz.x, sz.y);
    glEnd();

    // text x
    glPushMatrix();
    glTranslatef(sx.x*1.5f, sx.y*1.5f, 0.0f);
    glScalef(1.0f/ratio, 1.0f, 1.0f);
    glColor3f(1.0f, 0.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.5f, 0.5f);
	glVertex2f(0.5f, -0.5f);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(0.5f, 0.5f);
    glEnd();
    glPopMatrix();

    //text y
    glPushMatrix();
    glTranslatef(sy.x*1.5f, sy.y*1.5f, 0.0f);
    glScalef(1.0f/ratio, 1.0f, 1.0f);
    glColor3f(0.0f, 1.0f, 0.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.5f, 0.5f);
	glVertex2f(0.0f, 0.25f);
	glVertex2f(0.5f, 0.5f);
	glVertex2f(0.0f, 0.25f);
	glVertex2f(0.0f, 0.25f);
	glVertex2f(0.0f, -0.5f);
    glEnd();
    glPopMatrix();

    //text z
    glPushMatrix();
    glTranslatef(sz.x*1.5f, sz.y*1.5f, 0.0f);
    glScalef(1.0f/ratio, 1.0f, 1.0f);
    glColor3f(0.0f, 0.0f, 1.0f);
    glBegin(GL_LINES);
    glVertex2f(-0.5f, 0.5f);
	glVertex2f(0.5f, 0.5f);
	glVertex2f(0.5f, 0.5f);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(-0.5f, -0.5f);
	glVertex2f(0.5f, -0.5f);
    glEnd();
    glPopMatrix();


    glLineWidth(1.0f);
    glPopMatrix();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
}

void Camera :: horizontalRotate()
{
    float theta = (-(mouse_pos.x - mouse_pre_pos.x)*100.0f);
    mat4 mat = translate(mat4(1.0), vec3(lookat));
    mat = rotate(mat, theta*pi<float>()/180.0f, vec3(world_up));
    mat = translate(mat, -vec3(lookat));

    eye = mat*eye;
}

void Camera :: verticalRotate()
{
	float theta = (mouse_pos.y - mouse_pre_pos.y)*100.0f;
    mat4 mat = translate(mat4(1.0f), vec3(lookat));
    mat = rotate(mat, theta*pi<float>()/180.0f, vec3(axis_u));
    mat = translate(mat, -vec3(lookat));

    eye = mat*eye;
    
    vec4 v1 = vec4(cross(vec3(world_up), vec3(axis_u)), 0.0f);
    vec4 v2 = eye - lookat;
    if(dot(v2,v1) > 0.0f ) {
        world_up.y = (world_up.y == 1.0f) ? -1.0f : 1.0f; 
    }
}

void Camera :: CameraRotate()
{
	horizontalRotate();
	verticalRotate();
}

//void Camera :: CameraZoom()
//{
//    float dir = mouse_pos.x - mouse_pre_pos.x;
//	eye -= lookat;
//
//	if(dir > 0.0f) {
//		eye += eye * M_ZOOM_PAR;
//		//lookat += eye * M_ZOOM_PAR; //this makes the lookat move along eye-lookat 
//	} else {
//		eye -= eye * M_ZOOM_PAR;
//		//lookat -= eye * M_ZOOM_PAR; //this makes the lookat move along eye-lookat 
//	}
//	eye += lookat;
//}

void Camera :: CameraPan()
{	
	eye.x += M_PAN_PAR*(viewMat[0][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[0][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
	eye.y += M_PAN_PAR*(viewMat[1][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[1][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
	eye.z += M_PAN_PAR*(viewMat[2][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[2][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
	
	lookat.x += M_PAN_PAR*(viewMat[0][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[0][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
	lookat.y += M_PAN_PAR*(viewMat[1][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[1][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
	lookat.z += M_PAN_PAR*(viewMat[2][0]*((mouse_pre_pos.x-mouse_pos.x) / 2) + viewMat[2][1]*((mouse_pre_pos.y-mouse_pos.y) / 2));
}

void Camera :: CameraPan_fp()
{
	eye.x += M_PAN_PAR_FP*(viewMat[0][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[0][2]*((key_pos.y-key_pre_pos.y) / 2));
	eye.y += M_PAN_PAR_FP*(viewMat[1][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[1][2]*((key_pos.y-key_pre_pos.y) / 2));
	eye.z += M_PAN_PAR_FP*(viewMat[2][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[2][2]*((key_pos.y-key_pre_pos.y) / 2));
	
	lookat.x += M_PAN_PAR_FP*(viewMat[0][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[0][2]*((key_pos.y-key_pre_pos.y) / 2));
	lookat.y += M_PAN_PAR_FP*(viewMat[1][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[1][2]*((key_pos.y-key_pre_pos.y) / 2));
	lookat.z += M_PAN_PAR_FP*(viewMat[2][0]*((key_pos.x-key_pre_pos.x) / 2) + viewMat[2][2]*((key_pos.y-key_pre_pos.y) / 2));
}

void Camera::CameraRotate_fp(int winW, int winH)
{
	float nh = 2.0f * near_plane * (float)tan(fovy*pi<float>()/180.0f*0.5f);	// near-plane height
    float len = length(eye-lookat);

	vec3 t;
	t.x = nh*M_ROTATE_PAR_FP*(mouse_pos.x-mouse_pre_pos.x)*len/(float)winH/near_plane;
	t.y = nh*M_ROTATE_PAR_FP*(mouse_pos.y-mouse_pre_pos.y)*len/(float)winH/near_plane;
    t.z = -len;
    t = normalize(t);
	t *= len;

    lookat.x += viewMat[0][0]*t.x + viewMat[0][1]*t.y+ viewMat[0][2]* t.z;
	lookat.y += viewMat[1][0]*t.x + viewMat[1][1]*t.y+ viewMat[1][2]* t.z;
	lookat.z += viewMat[2][0]*t.x + viewMat[2][1]*t.y+ viewMat[2][2]* t.z;

	vec4 v = lookat - eye;
    v = normalize(v);
	v *= len;
	lookat = eye + v;
}


void Camera::GetCamCS()
{
	axis_n = eye - lookat;
    axis_n = normalize(axis_n);
	axis_u = vec4(cross(vec3(world_up), vec3(axis_n)), 0.0f);
    axis_u = normalize(axis_u);
    axis_v = vec4(cross(vec3(axis_n), vec3(axis_u)), 0.0f);
	axis_v = normalize(axis_v);

    //float t[16];
    //glMatrixMode(GL_MODELVIEW);
	//glPushMatrix();
	//glGetFloatv(GL_MODELVIEW_MATRIX, t);
	//glPopMatrix();
    //viewMat = make_mat4(t);

    //glMatrixMode(GL_PROJECTION);
	//glPushMatrix();
	//glGetFloatv(GL_PROJECTION_MATRIX, t);
	//glPopMatrix();
    //projMat = make_mat4(t);
}

void Camera::GetViewFrustum()
{
	// compute the center of the near and far planes
	vec4 nc = eye - axis_n * near_plane;
	vec4 fc = eye - axis_n * far_plane;


	// compute width and height of the near and far plane
	float tang = (float)tan(fovy*3.14159265f/180.0f*0.5f);
	float nh = 2.0f*near_plane*tang;	// near-plane height
	float nw = nh*aspect;			    // near-plane width

	float fh = 2.0f*far_plane*tang;		// far-plane height
	float fw = fh*aspect;			    // far-plane width

	//compute the 8 conners of the frustum 
	ntl = nc + axis_v*nh*0.5f - axis_u*nw*0.5f;
	ntr = nc + axis_v*nh*0.5f + axis_u*nw*0.5f;
	nbl = nc - axis_v*nh*0.5f - axis_u*nw*0.5f;
	nbr = nc - axis_v*nh*0.5f + axis_u*nw*0.5f;

    ftl = fc + axis_v*fh*0.5f - axis_u*fw*0.5f;
	fbr = fc - axis_v*fh*0.5f + axis_u*fw*0.5f;
	ftr = fc + axis_v*fh*0.5f + axis_u*fw*0.5f;
	fbl = fc - axis_v*fh*0.5f - axis_u*fw*0.5f;
}

void Camera::drawFrustum()
{
    glUseProgram(0);
	glDisable(GL_LIGHTING);
	glLineWidth(1.5f);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//near plane
    glColor3f(1.0f, 0.7f, 1.0f);
	glBegin(GL_LINE_LOOP);
		glVertex3f(ntl.x, ntl.y, ntl.z);
		glVertex3f(ntr.x, ntr.y, ntr.z);
		glVertex3f(nbr.x, nbr.y, nbr.z);
		glVertex3f(nbl.x, nbl.y, nbl.z);
	glEnd();

	//far plane
	glBegin(GL_LINE_LOOP);
		glVertex3f(ftr.x, ftr.y, ftr.z);
		glVertex3f(ftl.x, ftl.y, ftl.z);
		glVertex3f(fbl.x, fbl.y, fbl.z);
		glVertex3f(fbr.x, fbr.y, fbr.z);
	glEnd();

	//bottom plane
	glBegin(GL_LINE_LOOP);
		glVertex3f(nbl.x, nbl.y, nbl.z);
		glVertex3f(nbr.x, nbr.y, nbr.z);
		glVertex3f(fbr.x, fbr.y, fbr.z);
		glVertex3f(fbl.x, fbl.y, fbl.z);
	glEnd();

	//top plane
	glBegin(GL_LINE_LOOP);
		glVertex3f(ntr.x, ntr.y, ntr.z);
		glVertex3f(ntl.x, ntl.y, ntl.z);
		glVertex3f(ftl.x, ftl.y, ftl.z);
		glVertex3f(ftr.x, ftr.y, ftr.z);
	glEnd();

	//left plane
	glBegin(GL_LINE_LOOP);
		glVertex3f(ntl.x, ntl.y, ntl.z);
		glVertex3f(nbl.x, nbl.y, nbl.z);
		glVertex3f(fbl.x, fbl.y, fbl.z);
		glVertex3f(ftl.x, ftl.y, ftl.z);
	glEnd();

	// right plane
	glBegin(GL_LINE_LOOP);
		glVertex3f(nbr.x, nbr.y, nbr.z);
		glVertex3f(ntr.x, ntr.y, ntr.z);
		glVertex3f(ftr.x, ftr.y, ftr.z);
		glVertex3f(fbr.x, fbr.y, fbr.z);
	glEnd();

	////eye-lookat (view direction)
	//glColor3f(1.0f, 0.6f, 0.5f);
	//glBegin(GL_LINES);
	//glVertex3f(eye.x, eye.y, eye.z);
	//glVertex3f(lookat.x, lookat.y, lookat.z);
	//glEnd();

    glLineWidth(1);
	glPopMatrix();
}