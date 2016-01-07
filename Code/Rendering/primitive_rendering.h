#ifndef primitive_rendering_h_
#define primitive_rendering_h_

#include "color_container.h"
#include "linear_algebra_templcode.h"
#include "linear_algebra.h"

class RenderPrimitives {
 public:
  RenderPrimitives();
  ~RenderPrimitives();
  
  void gl_draw_arrow(const Vector3f &pos,
    const Vector3f &direc,
    float arrow_length,
    float arrow_width,
    GLUquadricObj* pQuadric);

  void gl_draw_arrow(GLUquadricObj* pQuadric, Vector3f &pos, Vector3f &direc, float width);
  void gl_draw_arrow(GLUquadricObj* pQuadric, Vector3d &pos, Vector3d &direc, float width);
  void gl_draw_sphere(const Vector3f &pos, float scale, GLUquadricObj* pQuadric);
  void gl_draw_sphere(const Vector3d& origin, float scale, GLUquadricObj* pQuadric);
  void gl_draw_cylinder(GLUquadricObj* pQuadric,
    const Vector3f& p1, const Vector3f& p2,
    float r1, float r2);
  void gl_draw_cylinder(GLUquadricObj* pQuadric, const Vector3d& p1, const Vector3d& p2,
    float r1, float r2);
  void gl_draw_correspondence(GLUquadricObj* pQuadric,
    const Vector3f &spos, const Vector3f &tpos,
    float fearad, float corresrad,
    ColorContainer &color);
  void gl_draw_correspondence(GLUquadricObj* pQuadric,
    const Vector3d &spos, const Vector3d &tpos,
    float fearad, float corresrad,
    ColorContainer	&color);
};

#endif