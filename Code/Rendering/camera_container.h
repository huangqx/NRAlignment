#ifndef camera_container_h_
#define camera_container_h_


#include "linear_algebra_templcode.h"
#include "linear_algebra.h"
#include "bounding_box.h"

struct CameraContainer {
 public:
	CameraContainer();
	~CameraContainer();

  void Init(BoundingBox &box);
  void Setup();
  void MouseMove(int nFlags, int cx, int cy);
  void Reshape(int cx, int cy);
  void Scale(int d);
  void Rotate(double a,double b);
  void Move(int x,int y);
  
  Vector3d	eye_point_pos;
  Vector3d	gaze_point_pos;
  Vector3d	up_direc_vec;
  Vector3d	front_direc_vec;
  double view_distance;
  /************************************************************************/
  /* View port
  */
  /************************************************************************/
  int width;
  int	height;
};

#endif