#ifndef helper_functions_h_
#define helper_functions_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"
#include <vector>
using namespace std;

void GaussianWeighting(const vector<Vector3f> &point_positions,
  const vector<bool> &is_center_flags,
  const double &sigma,
  vector<float> *point_weights);

void FPSClustering(const vector<Vector3f> &point_normals,
  const vector<Vector3f> &point_colors,
  const double &weight_color,
  const double &cluster_size,
  vector<vector<int>> *clusters);

void ExtrinsicFPS(const vector<Vector3f> &point_poss,
  const double &sampling_density,
  vector<int> *sample_indices);

#endif