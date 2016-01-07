#include "pairwise_rigid_alignment.h"
#include "dynamic_linear_algebra_templcode.h"
#include "dynamic_linear_algebra.h"
#include <algorithm>

void PairwiseRigidAlign::Compute(const TriangleMesh &fixedSurf,
  const TriangleMesh &movingSurf,
  const PairwiseRigidAligPara &para,
  Affine3d *rigidTransform) {
  vector<int> sampleIds;
  Sampling(movingSurf, para.numSamples, &sampleIds);
  InitializeANN(fixedSurf, movingSurf, para.weightColor);
  
  double sceneSize = static_cast<float> (sqrt(movingSurf.GetBoundingBox()->size.getSqrNorm())
    + sqrt(fixedSurf.GetBoundingBox()->size.getSqrNorm()))/2.0;

  for (int levelId = 0; levelId < para.numLevels; ++levelId) {
    double t = levelId/(para.numLevels-1.0);
    double averagedCorrDis = exp((1-t)*log(para.initialAverageCorresDis)
      + t*log(para.finalAverageCorresDis))*sceneSize;

    for (int iteration = 0; iteration < para.numIterations; ++iteration) {
      OneIteration(fixedSurf, movingSurf, sampleIds,
        para.weightPoint2PlaneDis,
        para.weightColor,
        averagedCorrDis,
        rigidTransform);
    }
  }
}

void PairwiseRigidAlign::OneIteration(const TriangleMesh &fixedSurf,
  const TriangleMesh &movingSurf,
  const vector<int> &sampleIds,
  const double &weightPoint2Plane,
  const double &weightColor,
  const double &averageCorrDis,
  Affine3d *rigidTransform) {
  Matrix6d hessian;
  Vector6d gradient;
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j) {
      hessian[i][j] = 0.0;
    }
    gradient[i] = 0.0;
  }

  double J1[6];
  Vector3d J2[6];
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 6; ++j)
      J2[j][i] = 0.0;
    J2[i][i] = 1.0;
  }

  Vector3d curPos, // The current position of each moving point
    footPos, // Target position of each moving point
    footNor; // Normal at the target position

  double sigma2 = averageCorrDis*averageCorrDis;
  double sumOfSqrDis = 0.0, sumOfWeights = 0.0;
  for (unsigned i = 0; i < sampleIds.size(); ++i) {
    const Vertex &v = (*movingSurf.GetVertexArray())[sampleIds[i]];
    curPos = (*rigidTransform)[0]
    + (*rigidTransform)[1]*v.ori_pos[0]
    + (*rigidTransform)[2]*v.ori_pos[1]
    + (*rigidTransform)[3]*v.ori_pos[2];
    for (int i = 0; i < 3; ++i) {
      queryPt_[i] = curPos[i];
      queryPt_[i+3] = weightColor*v.color[i];
    }
    kdTree_fixed_->annkSearch(
      queryPt_, 
      1, 
      nnIdx_, 
      dists_,
      0.0);

    const Vertex &foot = (*fixedSurf.GetVertexArray())[nnIdx_[0]];
    for (int i = 0; i < 3; ++i) {
      footPos[i] = foot.ori_pos[i];
      footNor[i] = foot.ori_nor[i];
    }

    // Project it back to determine the disVec
    Vector3d disVec = footPos - curPos;
    double point2PlaneDis = disVec*footNor;
    double sqrPoint2PointDis = disVec.getSqrNorm();
    if (0) {
      Vector3d temp;
      for (int i = 0; i < 3; ++i)
        temp[i] = (foot.ori_pos[i] - (*rigidTransform)[0][i]);
      
      for (int i = 0; i < 3; ++i) {
        queryPt_[i] = (*rigidTransform)[i+1][0]*temp[0] +(*rigidTransform)[i+1][1]*temp[1] +(*rigidTransform)[i+1][2]*temp[2];
        queryPt_[i+3] = weightColor*foot.color[i];
      }
      kdTree_moving_->annkSearch(
        queryPt_, 
        1, 
        nnIdx_, 
        dists_,
        0.0);

      for (int i = 0; i < 3; ++i)
        temp[i] = v.ori_pos[i] -  (*movingSurf.GetVertexArray())[nnIdx_[0]].ori_pos[i];

      sqrPoint2PointDis = temp.getSqrNorm();
    }

    
    double corrWeight = exp(-sqrPoint2PointDis/2/sigma2);

    sumOfSqrDis += sqrPoint2PointDis*corrWeight;
    sumOfWeights += corrWeight;

    Vector3d buf = curPos.crossProduct(footNor);
    for (int i = 0; i < 3; ++i) {
      J1[i] = footNor[i];
      J1[i+3] = buf[i];
    }
    J2[4][0] = curPos[2];
    J2[5][0] = -curPos[1];
    J2[3][1] = -curPos[2];
    J2[5][1] = curPos[0];
    J2[3][2] = curPos[1];
    J2[4][2] = -curPos[0];
    for (int i = 0; i < 6; ++i) {
      for (int j = 0; j < 6; ++j) {
        hessian[i][j] += (J1[i]*J1[j]*weightPoint2Plane
          + (1-weightPoint2Plane)*(J2[i]*J2[j]))*corrWeight;
      }
      gradient[i] += (J1[i]*point2PlaneDis*weightPoint2Plane
        + (1-weightPoint2Plane)*(J2[i]*disVec))*corrWeight;
    }
  }
  // solve the linear system
  double velocity[6];
  Solve6x6(hessian, gradient, velocity);
  
  Affine3d newMotion(velocity);
  *rigidTransform = newMotion*(*rigidTransform);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 4; ++j) {
      printf("%f ", (*rigidTransform)[j][i]);
    }
    printf("\n");
  }
  printf("\n");
}

void PairwiseRigidAlign::Sampling(const TriangleMesh &movingSurf,
  const int &numSamples,
  vector<int> *sampleIds) {
  const Vector3d &bBoxSize = movingSurf.GetBoundingBox()->size;
  double sizeMax = max(bBoxSize[0],
    max(bBoxSize[1], bBoxSize[2]));
  double sizeMin = min(bBoxSize[0],
    min(bBoxSize[1], bBoxSize[2]));
  double sizeMid = bBoxSize[0]+bBoxSize[1]+bBoxSize[2]
  - sizeMax - sizeMin;
  double gridSize = sqrt(sizeMax*sizeMin/numSamples);
  Vector3d lowerConer = movingSurf.GetBoundingBox()->center_point;
  int dimX = static_cast<int> (bBoxSize[0]/gridSize) + 1,
    dimY = static_cast<int> (bBoxSize[1]/gridSize) + 1,
    dimZ = static_cast<int> (bBoxSize[2]/gridSize) + 1;
  lowerConer[0] -= dimX*gridSize/2;
  lowerConer[1] -= dimY*gridSize/2;
  lowerConer[2] -= dimZ*gridSize/2;

  vector<vector<int>> pointInCells;
  pointInCells.resize(dimX*dimY*dimZ);
  for (unsigned v_id = 0;
    v_id < movingSurf.GetVertexArray()->size();
    ++v_id) {
    const Vertex &v = (*movingSurf.GetVertexArray())[v_id];
    int coordX = static_cast<int> ((v.ori_pos[0] - lowerConer[0])/gridSize),
      coordY = static_cast<int> ((v.ori_pos[1] - lowerConer[1])/gridSize),
      coordZ = static_cast<int> ((v.ori_pos[2] - lowerConer[2])/gridSize);

    int cellId = (coordX*dimY+ coordY)*dimZ + coordZ;
    pointInCells[cellId].push_back(v_id);
  }

  for (unsigned i = 0; i < pointInCells.size(); ++i) {
    if (pointInCells[i].size() > 0)
      sampleIds->push_back(pointInCells[i][0]);
  }
  sort(sampleIds->begin(), sampleIds->end());
}

void PairwiseRigidAlign::InitializeANN(const TriangleMesh &fixedSurf,
  const TriangleMesh &movingSurf,
  const double &weightColor) {
  ClearANN();
  dataPts_fixed_ = annAllocPts(fixedSurf.GetVertexArray()->size(), 6);
  for (unsigned v_id = 0; v_id < fixedSurf.GetVertexArray()->size();
    ++v_id) {
    const Vertex &v = (*fixedSurf.GetVertexArray())[v_id];
    for (int i = 0; i < 3; i++) {
      dataPts_fixed_[v_id][i] = v.ori_pos[i];
      dataPts_fixed_[v_id][i+3] = v.color[i]*weightColor;
    }
  }
  kdTree_fixed_ = new ANNkd_tree(dataPts_fixed_,
    fixedSurf.GetVertexArray()->size(), 6);


  dataPts_moving_ = annAllocPts(movingSurf.GetVertexArray()->size(), 6);
  for (unsigned v_id = 0; v_id < movingSurf.GetVertexArray()->size();
    ++v_id) {
    const Vertex &v = (*movingSurf.GetVertexArray())[v_id];
    for (int i = 0; i < 3; i++) {
      dataPts_moving_[v_id][i] = v.ori_pos[i];
      dataPts_moving_[v_id][i+3] = v.color[i]*weightColor;
    }
  }
  kdTree_moving_ = new ANNkd_tree(dataPts_moving_,
    movingSurf.GetVertexArray()->size(), 6);


  queryPt_ = annAllocPt(6);
  nnIdx_ = new ANNidx[1];
  dists_ = new ANNdist[1];
}

void PairwiseRigidAlign::ClearANN() {
  if (dists_ != NULL) {
    delete []dists_;
    dists_ = NULL;
  }
  if (nnIdx_ != NULL) {
    delete []nnIdx_;
    nnIdx_ = NULL;
  }
  if (queryPt_ != NULL) {
    annDeallocPt(queryPt_);
    queryPt_ = NULL;
  }
  if (kdTree_fixed_ != NULL) {
    delete kdTree_fixed_;
    kdTree_fixed_ = NULL;
  }
  if (kdTree_moving_ != NULL) {
    delete kdTree_moving_;
    kdTree_moving_ = NULL;
  }
  if (dataPts_fixed_ != NULL) {
    annDeallocPts(dataPts_fixed_);
    dataPts_fixed_ = NULL;
  }
  if (dataPts_moving_ != NULL) {
    annDeallocPts(dataPts_moving_);
    dataPts_moving_ = NULL;
  }
}

bool PairwiseRigidAlign::Solve6x6(const Matrix6d &hessian, Vector6d &gradient,
  double *velocity) {
  // Using LLT factorization to solve the symmetric linear system
  const double fTolerance = 1e-20;
  double afV[6], Lower[6][6];
  for (int i = 0; i < 6; ++i) {
    for (int j = 0; j < 6; ++j)
      Lower[i][j] = hessian[i][j];
    velocity[i] = gradient[i];
  }

  for (int i1 = 0; i1 < 6; ++i1) {
    for (int i0 = 0; i0 < i1; ++i0)
      afV[i0] = Lower[i1][i0]*Lower[i0][i0];

    afV[i1] = Lower[i1][i1];
    for (int i0 = 0; i0 < i1; ++i0)
      afV[i1] -= Lower[i1][i0]*afV[i0];

    Lower[i1][i1] = afV[i1];
    if (fabs(afV[i1]) <= fTolerance) //singular
      return false;

    double fInv = 1.0f/afV[i1];
    for (int i0 = i1+1; i0 < 6; ++i0) {
      for (int i2 = 0; i2 < i1; ++i2)
        Lower[i0][i1] -= Lower[i0][i2]*afV[i2];
      Lower[i0][i1] *= fInv;
    }
  }

  // Solve Ax = B.
  // Forward substitution
  for (int i0 = 0; i0 < 6; ++i0) {
    for (int i1 = 0; i1 < i0; ++i1)
      velocity[i0] -= Lower[i0][i1]*velocity[i1];
  }

  // Diagonal division:  Let y = L^t x, then Dy = z.  Algorithm stores
  // y terms in B vector.
  for (int i0 = 0; i0 < 6; ++i0) {
    if (fabs(Lower[i0][i0]) <= fTolerance )
      return false;
    velocity[i0] /= Lower[i0][i0];
  }

  // Back substitution:  Solve L^t x = y.  Algorithm stores x terms in
  // B vector.
  for (int i0 = 4; i0 >= 0; i0--) {
    for (int i1 = i0+1; i1 < 6; ++i1)
      velocity[i0] -= Lower[i1][i0]*velocity[i1];
  }
  return true;
}