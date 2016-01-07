#ifndef scene_h_
#define scene_h_

#include "stdafx.h"
#include <vector>
using	namespace	std;

#include "display.h"

#define PICK_NONE 0
#define PICK_FACE	1

class Scene3Dim {
 public:
  Scene3Dim();
  ~Scene3Dim();
  
  int OnCreate(HWND &hWnd);
  void DrawScene(HWND &hWnd,CPaintDC &dc);
  void ViewAll(bool check_first = true);
  void OnSize(HWND &hWnd, int cx, int cy);
  void OnDestroy();
  bool OnRenderVisualchooser();

  void	BeginPick();
  int		EndPick(int x,int y);

  Display3Dim *GetDisplay() {
    return &display_;
  }

  // Return true if we want to change the guided vector field
  bool Pick(int x, int y);
  bool PickPoint(int n, UINT *pLong);

  void GenerateHandle(int px, int py);
  void AddHandle(int n, UINT *pLong);
  void RemoveHandle(int px, int py);
  void DeleteHandle(int n, UINT *pLong);
  void PickHandle(int px, int py, int &m_flag);
  void SetPickedHandle(int n, UINT *pLong);

 private:
	///Mesh Rendering
  void HandleMouseButton(int x, int y);
  bool SetWindowPixelFormat(HDC hDC);
  bool CreateViewGLContext(HDC hDC);
  void InitProjection();

 private:
  /// Rendering primitives
  Display3Dim	display_;
  HWND handle_wnd;
  
  // mouse
  bool left_buttion_down_;
  bool right_buttion_down_;
  CPoint left_down_position_;
  CPoint right_down_position;
  bool moving_;
  // OpenGL
  HGLRC	gl_context;
  int	gl_pixel_index;

  //picking
  UINT m_PickNum[1];
  GLubyte m_Pick[1][1][3];
  BYTE m_PickMode;
};

#endif