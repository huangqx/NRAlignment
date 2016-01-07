#ifndef patch_generator_h_
#define patch_generator_h_

#include "triangle_mesh.h"
#include "patch_container.h"

#include <vector>
using namespace std;

// Partition each scan into a set of overlapping patches
// with equal area. The basic idea is to first compute a 
// set of uniformly distributed samples. Each sample corresponds
// to a patch.
class PatchGenerator {
 public:
  PatchGenerator() {
  }
  ~PatchGenerator() {
  }
  // Main function, the result is stored in patch_container
  void Compute(const TriangleMesh &mesh, //Input scan
    const double &patch_radius, //patch radius
    const double &suppert_size, // patch support region size
    PatchContainer *patch_container);
 private:
  // Generate patches
  void ExtrinsicPatchGeneration(const vector<Vector3f> &point_positions,
    const double &patch_raidus,
    const double &suppert_size, // patch support region size
    vector<SurfacePatch> *patches);
  // Estimate the initial rigid motion associated with each patch.
  // This is casted as a point-point registration problem with known
  // correspondences
  void InitializePatchPose(const TriangleMesh &mesh,
    SurfacePatch *patch);
};

#endif