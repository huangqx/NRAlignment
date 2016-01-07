#ifndef light_h_
#define light_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"

struct Light3Dim {
 public:
  Light3Dim() {
  }
  ~Light3Dim() {
  }

	/// The light position
  float position[4];
	float	ambient[4];
	float	diffuse[4];
	float	specular[4];
};
#endif