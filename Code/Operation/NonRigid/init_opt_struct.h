#ifndef init_opt_struct_h_
#define init_opt_struct_h_

#include "triangle_mesh.h"
#include "patch_container.h"
#include "data_container.h"
#include "opt_struct_container.h"

#include <vector>
using namespace std;

class InitOptStruct {
 public:
  InitOptStruct() {
  }
  ~InitOptStruct() {
  }
  void SmoothnessTerm(const TriangleMesh &mesh,
    const PatchContainer &patch_container,
    OptStructContainer *opt_struct);

  // Used in multiple non-rigid registration
  void DataTerm(const ShapeContainer &moving_scan,
    const PatchContainer &deform_struct,
    const PointCloud &fixed_surf,
    const vector<int> &foot_point_indices,
    const double &weightPointPlaneDis,
    OptStructContainer *opt_struct);

  // Used in pair-wise non-rigid registration
  void DataTerm(const ShapeContainer &moving_scan,
    const PatchContainer &deform_struct,
    const vector<int> &sample_indices,
    const vector<Vector3f> &fixed_poss, // fixed scans
    const vector<Vector3f> &fixed_nors, // 
    const vector<AssociWeight> &corres, // nearest neighbor weights
    const double &weightPointPlaneDis,
    OptStructContainer *opt_struct);
};

#endif