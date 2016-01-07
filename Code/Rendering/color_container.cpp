#include "stdafx.h"
#include <math.h>
#include "color_container.h"

ColorContainer::ColorContainer() {
	surface_material	= MATERIAL_SILVER;
	corres_material	= MATERIAL_LIGHT_BLUE;
	feature_material	= MATERIAL_RUDY;

	back_color[0] = back_color[1] = back_color[2] = 1.f;
	mesh_color[0] = mesh_color[1] = mesh_color[2] = 0.2f;
	edge_color[0] = edge_color[1] = edge_color[2] = 0.4f;
	vertex_color[0] = vertex_color[1] = vertex_color[2] = 0.6f;
	direc_color[0] = direc_color[1] = direc_color[2] = 0.f;
	direc_color[0] = 1.f;

  code_background = static_cast<int>(255*back_color[0])*65536
    + static_cast<int>(255*back_color[1])*256
    + static_cast<int>(255*back_color[2]);
}

ColorContainer::~ColorContainer() {
}

void ColorContainer::SetColor(const float &val) {
	float r, g, b;
	HSVtoRGB(val, &r, &g, &b);
	glColor3f(r, g, b);
}

void ColorContainer::HSVtoRGB(const float &weight0,
                               float *x,
                               float *y,
                               float *z) {
  float weight = min(1.f, max(0.f, weight0));
	*x = 240*weight;
	*y = 1;
	*z = 1;
	float h = *x;
	float s = *y;
	float v = *z;
	float *rr = x;
	float *gg = y;
	float *bb = z;
	int i;
	float f, p, q, t;

	if( s == 0 ) {
		// achromatic (grey)
		*rr = *gg = *bb = v;
		return;
	}

	h /= 60;       // sector 0 to 5
	i = (int)floor( h );
	f = h - i;        // factorial part of h
	p = v * ( 1 - s );
	q = v * ( 1 - s * f );
	t = v * ( 1 - s * ( 1 - f ) );

	switch( i ) {
	  case 0:
		  *rr = v;
		  *gg = t;
		  *bb = p;
		  break;
	  case 1:
		  *rr = q;
		  *gg = v;
		  *bb = p;
		  break;
	  case 2:
		  *rr = p;
		  *gg = v;
		  *bb = t;
		  break;
	  case 3:
		  *rr = p;
		  *gg = q;
		  *bb = v;
		  break;
	  case 4:
		  *rr = t;
		  *gg = p;
		  *bb = v;
		  break;
	  default:    // case 5:
		  *rr = v;
		  *gg = p;
		  *bb = q;
		  break;
	}
}

void ColorContainer::EnableColorMaterial() {
	GLfloat shin[] = {100.0f};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shin);
	GLfloat spec[] = {0.1f,0.1f,0.1f,1.0f};
	GLfloat amb[] = {0,0,0,1};
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR , spec);
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, amb);
	glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
	glShadeModel(GL_SMOOTH);
}

void ColorContainer::DisableColorMaterial() {
	glDisable(GL_COLOR_MATERIAL);
}

void ColorContainer::ChangeMaterial(int material, bool front) {
	float	ambient[]  = {0.0f,0.0f,0.0f,1.0f};
	float	diffuse[]  = {0.0f,0.0f,0.0f,1.0f};
	float	specular[]  = {0.0f,0.0f,0.0f,1.0f};
	float	emission[]  = {0.3f,0.3f,0.3f,1.0f};
	float shininess[] = {0.0f};

	// Change
	switch (material) {
  case MATERIAL_SILVER:
		ambient[0] = 0.19225f;	ambient[1] = 0.19225f;	ambient[2] = 0.19225f;	ambient[3] = 1.0f;
		diffuse[0] = 0.50754f;	diffuse[1] = 0.50754f;	diffuse[2] = 0.50754f;	diffuse[3] = 1.0f;
		specular[0] = 0.508273f;specular[1] = 0.508273f;specular[2] = 0.508273f;specular[3] = 1.0f;
		shininess[0] = 51.2f;
		break;

	case MATERIAL_GOLD:
		ambient[0] = 0.24725f;	ambient[1] = 0.1995f;	ambient[2] = 0.0745f;	ambient[3] = 1.0f;
		diffuse[0] = 0.75164f;	diffuse[1] = 0.60648f;	diffuse[2] = 0.22648f;	diffuse[3] = 1.0f;
		specular[0] = 0.928281f;specular[1] = 0.855802f;specular[2] = 0.666065f;specular[3] = 1.0f;
		shininess[0] = 51.2f;
		break;

	case MATERIAL_JADE:
		ambient[0] = 0.135f;	ambient[1] = 0.2225f;	ambient[2] = 0.1575f;	ambient[3] = 1.0f;
		diffuse[0] = 0.54f;		diffuse[1] = 0.89f;		diffuse[2] = 0.63f;		diffuse[3] = 1.0f;
		specular[0] = 0.316228f;specular[1] = 0.316228f;specular[2] = 0.316228f;specular[3] = 1.0f;
		shininess[0] = 12.8f;
		break;

	case MATERIAL_LIGHT_BLUE:
		ambient[0] = 0.0f;		ambient[1] = 0.5f;		ambient[2] = 0.75f;		ambient[3] = 1.0f;
		diffuse[0] = 0.0f;		diffuse[1] = 0.5f;		diffuse[2] = 1.0f;		diffuse[3] = 1.0f;
		specular[0] = 0.75f;	specular[1] = 0.75f;	specular[2] = 0.75f;	specular[3] = 1.0f;
		shininess[0] = 64.0f;
		break;
	case MATERIAL_EMERALD:
		ambient[0] = 0.0215f;	ambient[1] = 0.1745f;	ambient[2] = 0.0215f;	ambient[3] = 1.0f;
		diffuse[0] = 0.07568f;	diffuse[1] = 0.61424f;	diffuse[2] = 0.07568f;	diffuse[3] = 1.0f;
		specular[0] = 0.633f;	specular[1] = 0.727811f;specular[2] = 0.633f;	specular[3] = 1.0f;
		shininess[0] = 76.8f;
		break;
	case MATERIAL_POLISHED_SILVER:
		ambient[0] = 0.23125f;	ambient[1] = 0.23125f;	ambient[2] = 0.23125f;	ambient[3] = 1.0f;
		diffuse[0] = 0.2775f;	diffuse[1] = 0.2775f;	diffuse[2] = 0.2775f;	diffuse[3] = 1.0f;
		specular[0] = 0.773911f;specular[1] = 0.773911f;specular[2] = 0.773911f;specular[3] = 1.0f;
		shininess[0] = 89.6f;
		break;

	case MATERIAL_CHROME:
		ambient[0] = 0.25f;		ambient[1] = 0.25f;		ambient[2] = 0.25f;		ambient[3] = 1.0f;
		diffuse[0] = 0.4f;		diffuse[1] = 0.4f;		diffuse[2] = 0.4f;		diffuse[3] = 1.0f;
		specular[0] = 0.774597f;specular[1] = 0.774597f;specular[2] = 0.774597f;specular[3] = 1.0f;
		shininess[0] = 76.8f;
		break;

	case MATERIAL_COPPER:
		ambient[0] = 0.19125f;	ambient[1] = 0.0735f;	ambient[2] = 0.0225f;	ambient[3] = 1.0f;
		diffuse[0] = 0.7038f;	diffuse[1] = 0.27048f;	diffuse[2] = 0.0828f;	diffuse[3] = 1.0f;
		specular[0] = 0.256777f;specular[1] = 0.137622f;specular[2] = 0.086014f;specular[3] = 1.0f;
		shininess[0] = 12.8f;
		break;

	case MATERIAL_POLISHED_GOLD:
		ambient[0] = 0.24725f;	ambient[1] = 0.2245f;	ambient[2] = 0.0645f;	ambient[3] = 1.0f;
		diffuse[0] = 0.34615f;	diffuse[1] = 0.3143f;	diffuse[2] = 0.0903f;	diffuse[3] = 1.0f;
		specular[0] = 0.797357f;specular[1] = 0.723991f;specular[2] = 0.208006f;specular[3] = 1.0f;
		shininess[0] = 83.2f;
		break;

	case MATERIAL_PEWTER:
		ambient[0] = 0.105882f;	ambient[1] = 0.058824f;	ambient[2] = 0.113725f;	ambient[3] = 1.0f;
		diffuse[0] = 0.427451f;	diffuse[1] = 0.470588f;	diffuse[2] = 0.541176f;	diffuse[3] = 1.0f;
		specular[0] = 0.333333f;specular[1] = 0.333333f;specular[2] = 0.521569f;specular[3] = 1.0f;
		shininess[0] = 9.84615f;
		break;

	case MATERIAL_OBSIDIAN:
		ambient[0] = 0.05375f;	ambient[1] = 0.05f;	ambient[2] = 0.06625f;	ambient[3] = 1.0f;
		diffuse[0] = 0.18275f;	diffuse[1] = 0.17f;	diffuse[2] = 0.22525f;	diffuse[3] = 1.0f;
		specular[0] = 0.332741f;specular[1] = 0.328634f;specular[2] = 0.346435f;specular[3] = 1.0f;
		shininess[0] = 38.4f;
		break;

	case MATERIAL_BLACK_PLASTIC:
		ambient[0] = 0.0f;		ambient[1] = 0.0f;		ambient[2] = 0.0f;		ambient[3] = 1.0f;
		diffuse[0] = 0.01f;		diffuse[1] = 0.01f;		diffuse[2] = 0.01f;		diffuse[3] = 1.0f;
		specular[0] = 0.5f;		specular[1] = 0.5f;		specular[2] = 0.5f;		specular[3] = 1.0f;
		shininess[0] = 32.0f;
		break;

	case MATERIAL_POLISHED_BRONZE:
		ambient[0] = 0.25f;		ambient[1] = 0.148f;	ambient[2] = 0.006475f;	ambient[3] = 1.0f;
		diffuse[0] = 0.4f;		diffuse[1] = 0.2368f;	diffuse[2] = 0.1036f;	diffuse[3] = 1.0f;
		specular[0] = 0.774597f;specular[1] = 0.458561f;specular[2] = 0.200621f;specular[3] = 1.0f;
		shininess[0] = 76.8f;
		break;

	case MATERIAL_POLISHED_COPPER:
		ambient[0] = 0.2295f;	ambient[1] = 0.08825f;	ambient[2] = 0.0275f;	ambient[3] = 1.0f;
		diffuse[0] = 0.5508f;	diffuse[1] = 0.2118f;	diffuse[2] = 0.066f;	diffuse[3] = 1.0f;
		specular[0] = 0.580594f;specular[1] = 0.223257f;specular[2] = 0.0695701f;specular[3] = 1.0f;
		shininess[0] = 51.2f;
		break;

	case MATERIAL_PEARL:
		ambient[0] = 0.25f;		ambient[1] = 0.20725f;	ambient[2] = 0.20725f;	ambient[3] = 1.0f;
		diffuse[0] = 1.0f;		diffuse[1] = 0.829f;	diffuse[2] = 0.829f;	diffuse[3] = 1.0f;
		specular[0] = 0.296648f;specular[1] = 0.296648f;specular[2] = 0.296648f;specular[3] = 1.0f;
		shininess[0] = 11.264f;
		break;

	case MATERIAL_RUDY:
		ambient[0] = 0.1745f;	ambient[1] = 0.01175f;	ambient[2] = 0.01175f;	ambient[3] = 1.0f;
		diffuse[0] = 0.61424f;	diffuse[1] = 0.04136f;	diffuse[2] = 0.04136f;	diffuse[3] = 1.0f;
		specular[0] = 0.727811f;specular[1] = 0.626959f;specular[2] = 0.626959f;specular[3] = 1.0f;
		shininess[0] = 76.8f;
		break;

	case MATERIAL_TURQUOISE:
		ambient[0] = 0.1f;		ambient[1] = 0.18725f;	ambient[2] = 0.1745f;	ambient[3] = 1.0f;
		diffuse[0] = 0.396f;	diffuse[1] = 0.74151f;	diffuse[2] = 0.69102f;	diffuse[3] = 1.0f;
		specular[0] = 0.297254f;specular[1] = 0.30829f;	specular[2] = 0.306678f;specular[3] = 1.0f;
		shininess[0] = 12.8f;
		break;

	case MATERIAL_BRASS:
		ambient[0] = 0.329412f;	ambient[1] = 0.223529f;	ambient[2] = 0.027451f;	ambient[3] = 1.0f;
		diffuse[0] = 0.780392f;	diffuse[1] = 0.268627f;	diffuse[2] = 0.113725f;	diffuse[3] = 1.0f;
		specular[0] = 0.992157f;specular[1] = 0.741176f;specular[2] = 0.807843f;specular[3] = 1.0f;
		shininess[0] = 27.8974f;
		break;

	case MATERIAL_GRASS:
		ambient[0] = 0.25f;		ambient[1] = 0.652647f;	ambient[2] = 0.254303f;	ambient[3] = 1.0f;
		diffuse[0] = 0.25f;		diffuse[1] = 0.652647f;	diffuse[2] = 0.254303f;	diffuse[3] = 1.0f;
		specular[0] = 0.478814f;specular[1] = 0.457627f;specular[2] = 0.5f;		specular[3] = 1.0f;
		shininess[0] = 25.0f;
		break;

	case MATERIAL_BRONZE:
		ambient[0] = 0.2125f;	ambient[1] = 0.1275f;	ambient[2] = 0.054f;	ambient[3] = 1.f;
		diffuse[0] = 0.714f;	diffuse[1] = 0.4284f;	diffuse[2] = 0.18144f;	diffuse[3] = 1.f;
		specular[0] = 0.393548f;specular[1] = 0.271906f;specular[2] = 0.166721f;specular[3] = 1.f;
		shininess[0] = 25.6f;
		break;
	}
	// apply
	if (front) {
		glMaterialfv( GL_FRONT, GL_AMBIENT,   ambient);
		glMaterialfv( GL_FRONT, GL_DIFFUSE,   diffuse);
		glMaterialfv( GL_FRONT, GL_SPECULAR,  specular);
		glMaterialfv( GL_FRONT, GL_SHININESS, shininess);
		glMaterialfv( GL_FRONT, GL_EMISSION,  emission);
	} else {
		glMaterialfv( GL_BACK, GL_AMBIENT,   ambient);
		glMaterialfv( GL_BACK, GL_DIFFUSE,   diffuse);
		glMaterialfv( GL_BACK, GL_SPECULAR,  specular);
		glMaterialfv( GL_BACK, GL_SHININESS, shininess);
		glMaterialfv( GL_BACK, GL_EMISSION,  emission);
	}
}