#ifndef pairwise_non_rigid_align_h_
#define pairwise_non_rigid_align_h_

#include "data_container.h"
#include "affine_transformation_templcode.h"
#include "affine_transformation.h"
#include "ANN.h"

#include <vector>
using namespace std;

struct PWNonRigidAlignPara {
 public:
  PWNonRigidAlignPara() {
    initialAverageCorresDis = 0.064;
    finalAverageCorresDis = 0.008;

    initialSmoothnessWeight = 1;
    finalSmoothnessWeight = 0.125;

    numLevels = 4;
    numIterations = 8;
    numSamples = 1024;
    weightPoint2PlaneDis = 0.5;
    weightColor = 10;
    weightNormal = 0.1;

    patchSize = 0.04;
    patchSupportSize = 2.5;
  }
  ~PWNonRigidAlignPara() {
  }
  double initialAverageCorresDis;
  double finalAverageCorresDis;

  double initialSmoothnessWeight;
  double finalSmoothnessWeight;

  double weightPoint2PlaneDis;
  double weightColor;
  double weightNormal;
  int numSamples;

  // The following two parameters are fixed usually
  int numLevels; // The number of coarse to fine levels
  // For each level, we use a fixed estimate of the averaged
  // distance between points and their target points
  int numIterations; // The number of iterations at each level

  double patchSize; // specifies the resolution of the deformation structure
  double patchSupportSize; // specifies the resolution of the deformation structure
};

class PWNonRigidAlign {
 public:
  PWNonRigidAlign() {
    dataPts_ = NULL;
    kdTree_ = NULL;
    queryPt_ = NULL;
    nnIdx_ = NULL;
    dists_ = NULL;
  }
  ~PWNonRigidAlign() {
    ClearANN();
  }
  void SetFixedScan(const vector<Vertex> &fixed_points);
  void Compute(const PWNonRigidAlignPara &para,
    ShapeContainer *movingSurf);
 private:
  void ClearANN();
  void InitializeANN(const double &weightColor,
    const double &weightNormal);
  void Sampling(const TriangleMesh &movingSurf,
    const int &numSamples,
    vector<int> *sampleIds);
  void NearestNeighborCorres(const TriangleMesh &movingSurf,
    const PatchContainer &patches,
    const vector<int> &sampleIds,
    const double &colorWeight,
    const double &normalWeight,
    const double &averagedCorrDis,
    vector<AssociWeight> *corres);
 private:
  // the 'scan' that is fixed. This 'scan' might come from multiple scans
  vector<Vector3f> fixed_poss_;
  vector<Vector3f> fixed_nors_;
  vector<Vector3f> fixed_colors_;

  // ANN for nearest neighbor search
  ANNpointArray	dataPts_;
  ANNkd_tree*	kdTree_;
  ANNpoint queryPt_;
  ANNidxArray	nnIdx_;
  ANNdistArray dists_;
};

#endif