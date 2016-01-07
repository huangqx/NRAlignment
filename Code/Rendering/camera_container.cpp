#include "stdafx.h"
#include "camera_container.h"

CameraContainer::CameraContainer() {
}

CameraContainer::~CameraContainer() {
}

void CameraContainer::Init(BoundingBox &box) {
  double	lengthOfDiagonal = sqrt(box.size.getSqrNorm());
  view_distance		= lengthOfDiagonal;
  gaze_point_pos[0]= box.center_point[0];
  gaze_point_pos[1]= box.center_point[1];
  gaze_point_pos[2]= box.center_point[2];
  eye_point_pos[0] = box.center_point[0];
  eye_point_pos[1] = box.center_point[1];
  eye_point_pos[2] = box.center_point[2] + lengthOfDiagonal;
  up_direc_vec[0] = 0.0;
  up_direc_vec[1] = 1.0;
  up_direc_vec[2] = 0.0;
  front_direc_vec[0] = 0.0;
  front_direc_vec[1] = 0.0;
  front_direc_vec[2] = 1.0;
}

void CameraContainer::Setup() {
  glLoadIdentity();
  gluLookAt(eye_point_pos[0], eye_point_pos[1], eye_point_pos[2],
    gaze_point_pos[0], gaze_point_pos[1], gaze_point_pos[2],
    up_direc_vec[0], up_direc_vec[1], up_direc_vec[2]);
}

void CameraContainer::MouseMove(int nFlags, int cx, int cy) {
  if(nFlags & MK_SHIFT)
    Scale(cx);
  else if(nFlags & MK_CONTROL)
    Move(cx,-cy);
  else if(::GetKeyState(VK_MENU) & 0xf0)
    Rotate(cx/300.f,-cy/300.0);
	//	m_nScale=eye_point_pos.length()/20.0;
}

void CameraContainer::Rotate(double a,double b) {
  Vector3d w = up_direc_vec.crossProduct(front_direc_vec);
  w /= sqrt(w*w);
  front_direc_vec = front_direc_vec*(float)cos(a)+w*(float)sin(a);
  
  w = up_direc_vec.crossProduct(front_direc_vec);
  w /= sqrt(w*w);
  front_direc_vec = front_direc_vec*(float)cos(b)+up_direc_vec*(float)sin(b);
  up_direc_vec = front_direc_vec.crossProduct(w);
  
  eye_point_pos = gaze_point_pos+front_direc_vec*(float)view_distance;
}

void CameraContainer::Scale(int d) {
	view_distance *= 1.0+(double)d/(double)width;
	eye_point_pos = gaze_point_pos + front_direc_vec * (float)view_distance;
}

void CameraContainer::Reshape(int cx, int cy) {
	if (cx>0 && cy>0) {
		width = cx;
		height = cy;

		if((cx<=0)||(cy<=0))
			return;

		width = cx;	height = cy;

		wglMakeCurrent(NULL, NULL);
		double aspect;
		aspect = (double)cx/(double)cy;
		glViewport(0,0,cx,cy);
		glTranslated(0.0f, 0.0f, -3.0f);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPerspective(45,aspect, 0.005f,1000.0f);
		glMatrixMode(GL_MODELVIEW);
		wglMakeCurrent(NULL,NULL);
	}
}

void CameraContainer::Move(int x,int y) {
	Vector3d w = up_direc_vec.crossProduct(front_direc_vec);
	Vector3d W = w*(float)view_distance*(float)x/(float)width;
	Vector3d U = up_direc_vec*(float)view_distance*(float)y/(float)width;
	eye_point_pos += W+U;
	gaze_point_pos += W+U;
}