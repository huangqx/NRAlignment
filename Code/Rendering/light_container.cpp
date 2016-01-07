#include "light_container.h"

LightContainer::LightContainer() {
  Initialize();
}

LightContainer::~LightContainer() {
}

void LightContainer::Initialize() {
  lights.resize(2);
  Light3Dim	&light0 = lights[0];
  Light3Dim	&light1 = lights[1];

  light0.position[0] = -400.7071f;	
  light0.position[1] = 0.7071f;
  light0.position[2] = 408.0f;
  light0.position[3] = 1.f;

  light0.ambient[0] = 0.4f;
  light0.ambient[1] = 0.4f;
  light0.ambient[2] = 0.4f;
  light0.ambient[3] = 1.f;

  light0.diffuse[0] = 0.3f;
  light0.diffuse[1] = 0.3f;
  light0.diffuse[2] = 0.3f;
  light0.diffuse[3] = 1.f;

  light0.specular[0] = 0.5f;
  light0.specular[1] = 0.5f;
  light0.specular[2] = 0.5f;
  light0.specular[3] = 1.f;

  light1.position[0] = 400.7071f;	
  light1.position[1] = 0.7071f;
  light1.position[2] = 408.0f;
  light1.position[3] = 1.f;

  light1.ambient[0] = 0.2f;
  light1.ambient[1] = 0.2f;
  light1.ambient[2] = 0.2f;
  light1.ambient[3] = 1.f;

  light1.diffuse[0] = 0.4f;
  light1.diffuse[1] = 0.4f;
  light1.diffuse[2] = 0.4f;
  light1.diffuse[3] = 1.f;

  light1.specular[0] = 0.5f;
  light1.specular[1] = 0.5f;
  light1.specular[2] = 0.5f;
  light1.specular[3] = 1.f;
}