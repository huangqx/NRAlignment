#include "scene.h"
#include "data_container.h"

Scene3Dim::Scene3Dim() {
  m_PickMode = PICK_NONE;
}

Scene3Dim::~Scene3Dim() {
}

int Scene3Dim::OnCreate(HWND &hWnd) {
  handle_wnd = hWnd;
  HDC hDC = ::GetDC(hWnd);

	if(!SetWindowPixelFormat(hDC))
		return -1;

	if(!CreateViewGLContext(hDC))
		return -1;

  // init rendering setting
	display_.Init();
	
	// init camera
	InitProjection();

	return 0;
}

bool	Scene3Dim::SetWindowPixelFormat(HDC hDC){
	PIXELFORMATDESCRIPTOR pixelDesc;

	pixelDesc.nSize = sizeof(PIXELFORMATDESCRIPTOR);
	pixelDesc.nVersion = 1;

	pixelDesc.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL |
		PFD_DOUBLEBUFFER | PFD_STEREO_DONTCARE;

	pixelDesc.iPixelType = PFD_TYPE_RGBA;
	pixelDesc.cColorBits = 32;
	pixelDesc.cRedBits = 8;
	pixelDesc.cRedShift = 16;
	pixelDesc.cGreenBits = 8;
	pixelDesc.cGreenShift = 8;
	pixelDesc.cBlueBits = 8;
	pixelDesc.cBlueShift = 0;
	pixelDesc.cAlphaBits = 0;
	pixelDesc.cAlphaShift = 0;
	pixelDesc.cAccumBits = 64;
	pixelDesc.cAccumRedBits = 16;
	pixelDesc.cAccumGreenBits = 16;
	pixelDesc.cAccumBlueBits = 16;
	pixelDesc.cAccumAlphaBits = 0;
	pixelDesc.cDepthBits = 32;
	pixelDesc.cStencilBits = 8;
	pixelDesc.cAuxBuffers = 0;
	pixelDesc.iLayerType = PFD_MAIN_PLANE;
	pixelDesc.bReserved = 0;
	pixelDesc.dwLayerMask = 0;
	pixelDesc.dwVisibleMask = 0;
	pixelDesc.dwDamageMask = 0;

	gl_pixel_index = ChoosePixelFormat(hDC,&pixelDesc);
	if(gl_pixel_index == 0) // Choose default
	{
		gl_pixel_index = 1;
		if(DescribePixelFormat(hDC,gl_pixel_index,
			sizeof(PIXELFORMATDESCRIPTOR),&pixelDesc)==0)
			return false;
	}

	if(!SetPixelFormat(hDC,gl_pixel_index,&pixelDesc))
		return false;

	return true;
}

bool	Scene3Dim::CreateViewGLContext(HDC hDC){
	gl_context = wglCreateContext(hDC);

	if(gl_context==NULL)
		return false;

	if(wglMakeCurrent(hDC,gl_context)==FALSE)
		return false;

	return true;	
}

void Scene3Dim::InitProjection() {
  CRect rect;
  HDC hDC = ::GetDC(handle_wnd);

	GetClientRect(handle_wnd, &rect);

	wglMakeCurrent(hDC,gl_context);

	// set viewport and camera
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	double ratio = double(rect.Width())/rect.Height();
	// Projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45,ratio,0.005,2000);
	glMatrixMode(GL_MODELVIEW);

	::ReleaseDC(handle_wnd,hDC);
}

void Scene3Dim::DrawScene(HWND &hWnd,CPaintDC &dc) {
	// Initialize
	HDC hDC = ::GetDC(hWnd);
	// wglMakeCurrent(hDC,gl_context);
	wglMakeCurrent(dc.m_ps.hdc,gl_context);
	
	display_.Draw();

	// Double buffer
	SwapBuffers(dc.m_ps.hdc);
	glFlush();

	// release
	::ReleaseDC(hWnd,hDC);
	wglMakeCurrent(NULL,NULL);
}


void Scene3Dim::BeginPick() {
  HDC hDC = ::GetDC(handle_wnd);
  wglMakeCurrent(hDC, gl_context);
  glPushAttrib(GL_COLOR_BUFFER_BIT);
  glClearColor(0.0,0.0,0.0,1.0);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPopAttrib();
  glLoadIdentity();
  Vector3d &eye_point = display_.GetCamera()->eye_point_pos;
  Vector3d &gaze_point = display_.GetCamera()->gaze_point_pos;
  Vector3d &up_vec = display_.GetCamera()->up_direc_vec;
  gluLookAt(eye_point[0], eye_point[1], eye_point[2],
    gaze_point[0], gaze_point[1], gaze_point[2],
    up_vec[0], up_vec[1], up_vec[2]);

  glPushAttrib(GL_LIGHTING_BIT);
  glDisable(GL_LIGHTING);
}

int Scene3Dim::EndPick(int x,int y) {
  glPopAttrib();
  int width = display_.GetCamera()->width;
  int height = display_.GetCamera()->height;
  
  if ((x < HALFPICKRANGE)
    || (y < HALFPICKRANGE)
    || (x > width-HALFPICKRANGE)
    || (y > height-HALFPICKRANGE))
    return 0;

  glFinish();
  glReadPixels(x,
    height - y,
    1, 1,
    GL_RGB,GL_UNSIGNED_BYTE,
    m_Pick);

  int i,j,k,n;
  UINT l;
  n=0;
  for(i=0;i<1;i++)
  {
    for(j=0;j<1;j++)
    {
      l=m_Pick[i][j][0]*65536+m_Pick[i][j][1]*256+m_Pick[i][j][2];
      if(l > 0 && l != display_.GetColor()->code_background)
      {
        if(n==0) {
          m_PickNum[n++]=l;
        } else {
          for (k=0; k<n; k++) {
            if(m_PickNum[k]==l)
              break;
          }
          if (m_PickNum[k] != l)
            m_PickNum[n++]=l;
        }
      }
    }
  }

  //	SwapBuffers(m_hDC);
  wglMakeCurrent(NULL,NULL);
  return n;
}

bool Scene3Dim::Pick(int x, int y) {

  BeginPick();
  int n = EndPick(x, y);
  return PickPoint(n, m_PickNum);
}

bool Scene3Dim::PickPoint(int n,UINT *pLong) {
  if (n == 0 || pLong[0] <= 0) {
    
    return false;
  } else {
    
    return false;
  }
  return false;
}

