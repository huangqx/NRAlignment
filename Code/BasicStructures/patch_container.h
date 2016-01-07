#ifndef patch_container_h_
#define patch_container_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"
#include "affine_transformation_templcode.h"
#include "affine_transformation.h"
#include <vector>
using namespace std;

struct AssociWeight {
  AssociWeight() {
    vertex_index = 0;
    weight = 1.f;
  }
  inline AssociWeight(int id, float w) {
    vertex_index = id;
    weight = w;
  }
  ~AssociWeight() {
  }
  const	inline bool operator < (const AssociWeight& aw) const {
    if(weight > aw.weight) {
      return true;
    } else if(weight < aw.weight) {
      return false;
    } else {
      return vertex_index < aw.vertex_index;
    }
  }
  int vertex_index;
  float weight;
};

struct SurfacePatch {
 public:
  SurfacePatch() {
  }
  ~SurfacePatch() {
  }
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);
  vector<AssociWeight> points;
  vector<int> kernel_region_indices;

  // Rigid motion associated with this patch
  Affine3d motion;
};

struct PatchContainer {
 public:
	PatchContainer() {
  }
	~PatchContainer() {
  }

	/************************************************************************/
	/* IO Operations
	*/
	/************************************************************************/
	void Read(FILE *file_ptr);
	void Write(FILE *file_ptr);

  vector<SurfacePatch> surface_patches;
};
#endif