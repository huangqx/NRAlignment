#ifndef pairwise_rigid_h_
#define pairwise_rigid_h_

#include "triangle_mesh.h"
#include "affine_transformation_templcode.h"
#include "affine_transformation.h"
#include "ANN.h"

#include <vector>
using namespace std;

struct PairwiseRigidAligPara {
 public:
  PairwiseRigidAligPara() {
    initialAverageCorresDis = 0.064;
    finalAverageCorresDis = 0.008;
    numLevels = 4;
    numIterations = 8;
    numSamples = 1024;
    weightPoint2PlaneDis = 0.5;
    weightColor = 10;
  }
  ~PairwiseRigidAligPara() {
  }
  double initialAverageCorresDis;
  double finalAverageCorresDis;
  double weightPoint2PlaneDis;
  double weightColor;
  int numSamples;

  // The following two parameters are fixed usually
  int numLevels; // The number of coarse to fine levels
  // For each level, we use a fixed estimate of the averaged
  // distance between points and their target points
  int numIterations; // The number of iterations at each level
};

class PairwiseRigidAlign {
 public:
  PairwiseRigidAlign() {
    dataPts_moving_ = NULL;
    dataPts_fixed_ = NULL;
    kdTree_moving_ = NULL;
    kdTree_fixed_ = NULL;
    queryPt_ = NULL;
    nnIdx_ = NULL;
    dists_ = NULL;
  }
  ~PairwiseRigidAlign() {
    ClearANN();
  }
  void Compute(const TriangleMesh &fixedSurf,
    const TriangleMesh &movingSurf,
    const PairwiseRigidAligPara &para,
    Affine3d *rigidTransform);
 private:
  void ClearANN();
  void InitializeANN(const TriangleMesh &fixedSurf,
    const TriangleMesh &movingSurf,
    const double &weightColor);
  void Sampling(const TriangleMesh &movingSurf,
    const int &numSamples,
    vector<int> *sampleIds);
  void OneIteration(const TriangleMesh &fixedSurf,
    const TriangleMesh &movingSurf,
    const vector<int> &sampleIds,
    const double &weightPoint2Plane,
    const double &weightColor,
    const double &averageCorrDis,
    Affine3d *rigidTransform);
  bool Solve6x6(const Matrix6d &hessian, Vector6d &gradient,
    double *velocity);
 private:
  // ANN for nearest neighbor search
  ANNpointArray	dataPts_moving_;
  ANNpointArray	dataPts_fixed_;
  ANNkd_tree*	kdTree_moving_;
  ANNkd_tree*	kdTree_fixed_;
  ANNpoint queryPt_;
  ANNidxArray	nnIdx_;
  ANNdistArray dists_;
};

#endif