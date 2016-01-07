#ifndef target_point_generator_h_
#define target_point_generator_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"
#include "data_container.h"
#include "octree.h"

#include <vector>
using namespace std;


struct RefSurfPara {
public:
  RefSurfPara() {
    gridRes = 0.01;
    halfWindowWidth = 1;
    weightColor = 1.f;
    clusterSize = 1.f;
    weightPointPlaneDis = 0.9;
  }
  ~RefSurfPara() {
  }
  double gridRes; // Determines the resolution of the reference surface
  int halfWindowWidth; //determines the moving window size
  double weightColor; // determines the weight of the color info in clustering
  double clusterSize; // determines the size of each cluster
  double weightPointPlaneDis; // a weight between 0 and 1 that blances
  // the point 2 point distance and the point to plane distance.
};

// Compute the reference surface from the input scans
// Find the target point of each scan point
class TargetPointGenerator {
 public:
  TargetPointGenerator() {
  }
  ~TargetPointGenerator() {
  }
  // Major function
  void Compute(const vector<ShapeContainer> &input_scans, //input scans
    const RefSurfPara &para, // Parameters 
    vector<Vertex> *foot_points, // The reference surface (as a point cloud)
    vector<vector<int>> *foot_point_indices);
  // the target point index of each scan point
 private:
  Octree3D* GenerateOctree(const vector<ShapeContainer> &input_scans,
    const double &gridRes);
  void InsertAPoint(const int &scan_index, const int &point_index,
    Node3D** current_node);
 private:
  int depth_;
  int max_depth_;
  int cell_offsets_[3];
};

#endif