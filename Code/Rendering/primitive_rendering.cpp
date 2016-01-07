#include "stdafx.h"
#include "primitive_rendering.h"

RenderPrimitives::RenderPrimitives() {
}

RenderPrimitives::~RenderPrimitives() {
}

void RenderPrimitives::gl_draw_arrow(const Vector3f &pos,
                                     const Vector3f &direc,
                                     float arrow_length,
                                     float arrow_width,
                                     GLUquadricObj* pQuadric) {
  if (arrow_width * 6.f > arrow_length)
    arrow_width = arrow_length/6.0f;

  gluSphere(pQuadric, 0.f, 20, 20);
  glPushMatrix();

  /// translate
  glTranslatef(pos[0], pos[1], pos[2]);
  /// rotate 
  float angle = acos(max(min(direc[2], 1.f), -1.f));
  double	PI = 3.1415926535897;
  glRotatef(float(-angle/PI*180.0), (float)direc[1], -(float)direc[0], 0.0f);
  gluSphere(pQuadric, arrow_width, 20, 20);
  gluCylinder(pQuadric,
    arrow_width,
    arrow_width,
    arrow_length - 5*arrow_width,
    20, 20);
  glTranslatef(0.f, 0.f, arrow_length - 6*arrow_width);
  glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);
  gluDisk(pQuadric, 0, 4*arrow_width, 20, 20);
  glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);

  gluCylinder(pQuadric, 4*arrow_width, 0, 6*arrow_width, 20, 20);

  glPopMatrix();
}

void RenderPrimitives::gl_draw_arrow(GLUquadricObj* pQuadric,
                                     Vector3f &pos,
                                     Vector3f &direc,
                                     float width) {
	float	len1 = 0.8f;
	float	len2 = 0.7f;

	Vector3f	v1 = pos, v2 = pos + direc*len1;
	gl_draw_cylinder(pQuadric, v1, v2, width, width);
	v1 = pos + direc*len2;
	v2 = pos + direc;
	gl_draw_cylinder(pQuadric, v1, v2, 1.5f*width, 0.0f);
}

void RenderPrimitives::gl_draw_arrow(GLUquadricObj* pQuadric,
                                     Vector3d &pos,
                                     Vector3d &direc,
                                     float width) {
	float	len1 = 0.8f;
	float	len2 = 0.7f;

	Vector3d	v1 = pos, v2 = pos + direc*len1;
	gl_draw_cylinder(pQuadric, v1, v2, width, width);
	v1 = pos + direc*len2;
	v2 = pos + direc;
	gl_draw_cylinder(pQuadric, v1, v2, 1.5f*width, 0.0f);
}

void	RenderPrimitives::gl_draw_cylinder(
  GLUquadricObj* pQuadric,
  const Vector3f &p1,
  const Vector3f &p2,
  float r1,
  float r2) {
	double	PI = 3.1415926535897;

	Vector3f	dev = p2 - p1;
	double angle = atan(sqrt(dev[0]*dev[0]+dev[1]*dev[1])/dev[2]);
	glPushMatrix();
	glTranslatef(p1[0], p1[1], p1[2]);
	if(angle>0)
		glRotatef(float(- angle/PI*180.0), (float)dev[1], -(float)dev[0], 0.0f);
	else
		glRotatef( float((1.0-angle/PI) * 180.0), (float)dev[1], -(float)dev[0], 0.0f);
	gluCylinder(pQuadric, r1, r2, sqrt(dev*dev), 20, 20);
	glPopMatrix();
}

void	RenderPrimitives::gl_draw_cylinder(
  GLUquadricObj* pQuadric,
  const Vector3d &p1,
  const Vector3d &p2,
  float r1,
  float r2) {
	double	PI = 3.1415926535897;

	Vector3d	dev = p2 - p1;
	double angle = atan(sqrt(dev[0]*dev[0]+dev[1]*dev[1])/dev[2]);
	glPushMatrix();
	glTranslated(p1[0], p1[1], p1[2]);
	if(angle>0)
		glRotatef(float(- angle/PI*180.0), (float)dev[1], -(float)dev[0], 0.0f);
	else
		glRotatef( float((1.0-angle/PI) * 180.0), (float)dev[1], -(float)dev[0], 0.0f);
	gluCylinder(pQuadric, r1, r2, sqrt(dev*dev), 20, 20);
	glPopMatrix();
}

void	RenderPrimitives::gl_draw_sphere(const Vector3f &pos,
                                       float radius,
                                       GLUquadricObj* pQuadric) {
	glPushMatrix();
	glTranslatef(pos[0], pos[1], pos[2]);
	gluSphere(pQuadric, radius, 20, 20);
	glPopMatrix();
}

void	RenderPrimitives::gl_draw_sphere(const Vector3d &pos,
                                       float radius,
                                       GLUquadricObj* pQuadric) {
	glPushMatrix();
	glTranslated(pos[0], pos[1], pos[2]);
	gluSphere(pQuadric, radius, 20, 20);
	glPopMatrix();
}

void	RenderPrimitives::gl_draw_correspondence(
  GLUquadricObj* pQuadric,
  const Vector3f &spos,
  const Vector3f &tpos,
  float fearad,
  float corresrad,
  ColorContainer	&color) {
	gluSphere(pQuadric, 0.f, 20, 20);
	Vector3f	direc = tpos - spos;
	float	sqrDis = direc.getSqrNorm();
	glPushMatrix();
	if (sqrDis < fearad*fearad) {
		/// Render features
		color.ChangeMaterial(color.feature_material, true);
		/// translate
		glTranslatef(spos[0], spos[1], spos[2]);
		gluSphere(pQuadric, fearad, 20, 20);
		glTranslatef(tpos[0]-spos[0],tpos[1]-spos[1], tpos[2]-spos[2]);
		gluSphere(pQuadric, fearad, 20, 20);
	} else {
		float dis = sqrt(sqrDis);
		/// translate
		glTranslatef(spos[0], spos[1], spos[2]);
		/// rotate 
		float angle = acos(direc[2]/dis);
		double	PI = 3.1415926535897;
		glRotatef(float(-angle/PI*180.0), (float)direc[1], -(float)direc[0], 0.0f);

		color.ChangeMaterial(color.feature_material, true);
		gluSphere(pQuadric, fearad, 20, 20);
		color.ChangeMaterial(color.corres_material, true);
		if (sqrDis < 25*fearad*fearad) {
			gluCylinder(pQuadric, corresrad, corresrad, dis, 40, 40);
			glTranslatef(0.f, 0.f, dis);
		} else{
			gluCylinder(pQuadric, corresrad, corresrad, dis-5*fearad, 40, 40);
			glTranslatef(0.f, 0.f, dis-5*fearad);
			glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);
			gluDisk(pQuadric, 0, fearad, 10, 10);
			glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);

			gluCylinder(pQuadric, fearad, 0, 4.25*fearad, 40, 40);
			glTranslatef(0.f, 0.f, 5*fearad);
		}
		color.ChangeMaterial(color.feature_material, true);
		gluSphere(pQuadric, fearad, 20, 20);
	}
	glPopMatrix();
}


void	RenderPrimitives::gl_draw_correspondence(
  GLUquadricObj* pQuadric,
  const Vector3d &spos,
  const Vector3d &tpos,
  float fearad,
  float corresrad,
  ColorContainer &color) {
	gluSphere(pQuadric, 0.f, 20, 20);
	Vector3d	direc = tpos - spos;
	float	sqrDis = float(direc.getSqrNorm());
	glPushMatrix();
	if (sqrDis < fearad*fearad){
		/// Render features
		color.ChangeMaterial(color.feature_material, true);
		/// translate
		glTranslated(spos[0], spos[1], spos[2]);
		gluSphere(pQuadric, fearad, 20, 20);
		glTranslated(tpos[0]-spos[0],tpos[1]-spos[1], tpos[2]-spos[2]);
		gluSphere(pQuadric, fearad, 20, 20);
	} else {
		float dis = sqrt(sqrDis);
		/// translate
		glTranslated(spos[0], spos[1], spos[2]);
		/// rotate 
		float angle = float(acos(direc[2]/dis));
		double	PI = 3.1415926535897;
		glRotatef(float(-angle/PI*180.0), (float)direc[1], -(float)direc[0], 0.0f);

		color.ChangeMaterial(color.feature_material, true);
		gluSphere(pQuadric, fearad, 20, 20);
		color.ChangeMaterial(color.corres_material, true);
		if(sqrDis < 25*fearad*fearad){
			gluCylinder(pQuadric, corresrad, corresrad, dis, 40, 40);
			glTranslatef(0.f, 0.f, dis);
		} else{
			gluCylinder(pQuadric, corresrad, corresrad, dis-5*fearad, 40, 40);
			glTranslatef(0.f, 0.f, dis-5*fearad);
			glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);
			gluDisk(pQuadric, 0, fearad, 10, 10);
			glRotatef(180, (float)direc[1], -(float)direc[0], 0.0f);

			gluCylinder(pQuadric, fearad, 0, 4.25*fearad, 40, 40);
			glTranslatef(0.f, 0.f, 5*fearad);
		}
		color.ChangeMaterial(color.feature_material, true);
		gluSphere(pQuadric, fearad, 20, 20);
	}
	glPopMatrix();
}