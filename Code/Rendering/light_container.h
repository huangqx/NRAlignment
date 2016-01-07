#ifndef light_container_h_
#define light_container_h_

#include <vector>
using	namespace	std;

#include "light.h"

struct LightContainer {
 public:
  LightContainer();
  ~LightContainer();
  
  void Initialize();
  vector<Light3Dim>	lights;
};
#endif