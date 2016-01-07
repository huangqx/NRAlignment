#include "pairwise_non_rigid_align.h"
#include "dynamic_linear_algebra_templcode.h"
#include "dynamic_linear_algebra.h"
#include "patch_container.h"
#include "patch_generator.h"
#include "opt_struct_container.h"
#include "init_opt_struct.h"
#include "gauss_newton_optimizer.h"
#include <algorithm>

void PWNonRigidAlign::Compute(const PWNonRigidAlignPara &para,
  ShapeContainer *movingSurf) {
  // Compute the deformation structure
  double sceneSize =
    static_cast<float> (sqrt(movingSurf->surface.GetBoundingBox()->size.getSqrNorm()));

  double weightColor = para.weightColor*sceneSize;
  double weightNormal = para.weightNormal*sceneSize;

  InitializeANN(weightColor, weightNormal);


  PatchContainer patches;
  PatchGenerator patch_generator;
  patch_generator.Compute(movingSurf->surface,
    para.patchSize*sceneSize,
    para.patchSupportSize,
    &patches);

  InitOptStruct opt_struct_generator;
  OptStructContainer opt_struct;
  // Pre-compute the smoothness term of the objective function
  opt_struct_generator.SmoothnessTerm(movingSurf->surface, patches,
      &opt_struct);


  vector<int> sampleIds;
  Sampling(movingSurf->surface, para.numSamples, &sampleIds);
  vector<AssociWeight> corres; // store the correspondences
  corres.resize(sampleIds.size());

  for (int levelId = 0; levelId < para.numLevels; ++levelId) {
    printf("Level %d: [", levelId);
    double t = levelId/(para.numLevels-1.0);
    double averagedCorrDis = exp((1-t)*log(para.initialAverageCorresDis)
      + t*log(para.finalAverageCorresDis))*sceneSize;

    double smoothnessWeight = exp((1-t)*log(para.initialSmoothnessWeight)
      + t*log(para.finalSmoothnessWeight))*sceneSize;
 
    // Solve the optimization problem for each scan
    // which minimizes the objective function that combines a data term(just computed)
    // and the smoothness term (which is pre-computed)
    for (int iter = 0; iter < para.numIterations; ++iter) {
      // Find nearest neighbor correspondences
      NearestNeighborCorres(movingSurf->surface,
        patches,
        sampleIds,
        weightColor,
        weightNormal,
        averagedCorrDis,
        &corres);

      // Generate the data term
      opt_struct_generator.DataTerm(*movingSurf,
        patches,
        sampleIds,
        fixed_poss_,
        fixed_nors_,
        corres,
        para.weightPoint2PlaneDis,
        &opt_struct);

      // Solve the induces correspondence problems
      GaussNewtonOptimizer gauss_newton_optimizer;
      gauss_newton_optimizer.SetSmoothnessWeight(smoothnessWeight);

      vector<Affine3d> poses;
      poses.resize(patches.surface_patches.size());
      for (unsigned i = 0; i < patches.surface_patches.size(); ++i)
        poses[i] = patches.surface_patches[i].motion;

      gauss_newton_optimizer.Compute(opt_struct, &poses);
      for (unsigned i = 0; i < patches.surface_patches.size(); ++i)
        patches.surface_patches[i].motion = poses[i];
      printf(".");
    }
    printf("]\n", levelId);
  }
  movingSurf->GenerateCurrentSurface(patches, true);
}


void PWNonRigidAlign::Sampling(const TriangleMesh &movingSurf,
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

void PWNonRigidAlign::InitializeANN(const double &weightColor,
  const double &weightNormal) {
  ClearANN();
  dataPts_ = annAllocPts(fixed_poss_.size(), 9);
  for (unsigned v_id = 0; v_id < fixed_poss_.size(); ++v_id) {
    for (int i = 0; i < 3; i++) {
      dataPts_[v_id][i] = fixed_poss_[v_id][i];
      dataPts_[v_id][i+3] = fixed_colors_[v_id][i]*weightColor;
      dataPts_[v_id][i+6] = fixed_nors_[v_id][i]*weightNormal;
    }
  }
  kdTree_ = new ANNkd_tree(dataPts_, fixed_poss_.size(), 9);

  queryPt_ = annAllocPt(9);
  nnIdx_ = new ANNidx[1];
  dists_ = new ANNdist[1];
}

void PWNonRigidAlign::ClearANN() {
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
  if (kdTree_ != NULL) {
    delete kdTree_;
    kdTree_ = NULL;
  }
  if (dataPts_ != NULL) {
    annDeallocPts(dataPts_);
    dataPts_ = NULL;
  }
}

void PWNonRigidAlign::NearestNeighborCorres(
  const TriangleMesh &movingSurf,
  const PatchContainer &patches,
  const vector<int> &sampleIds,
  const double &colorWeight,
  const double &normalWeight,
  const double &averageCorrDis,
  vector<AssociWeight> *corres) {
  vector<int> vertex_sIds;
  vertex_sIds.resize(movingSurf.GetVertexArray()->size());
  for (unsigned v_id = 0; v_id < vertex_sIds.size(); ++v_id)
    vertex_sIds[v_id] = -1;
  for (unsigned s_id = 0; s_id < sampleIds.size(); ++s_id)
    vertex_sIds[sampleIds[s_id]] = s_id;

  vector<Vector3f> cur_sample_poss, cur_sample_nors;
  cur_sample_poss.resize(sampleIds.size());
  cur_sample_nors.resize(sampleIds.size());

  for (unsigned patch_id = 0;
    patch_id < patches.surface_patches.size();
    ++patch_id) {
    const SurfacePatch &patch = patches.surface_patches[patch_id];
    const Affine3d &aff = patch.motion;
    for (unsigned i = 0; i < patch.kernel_region_indices.size(); ++i) {
      int vertex_index = patch.kernel_region_indices[i];
      int sample_index = vertex_sIds[vertex_index];
      if (sample_index < 0)
        continue;

      const Vertex &vertex = (*movingSurf.GetVertexArray())[vertex_index];
      Vector3d transformed_pos = aff[1]*vertex.ori_pos[0]
      + aff[2]*vertex.ori_pos[1] + aff[3]*vertex.ori_pos[2] + aff[0];
      Vector3d transformed_nor = aff[1]*vertex.ori_nor[0]
      + aff[2]*vertex.ori_nor[1] + aff[3]*vertex.ori_nor[2];
      for (int k = 0; k < 3; ++k) {
        cur_sample_poss[sample_index][k] =
        static_cast<float> (transformed_pos[k]);
        cur_sample_nors[sample_index][k] =
          static_cast<float> (transformed_nor[k]);
      }
    }
  }

  double sigma2 = averageCorrDis*averageCorrDis;
  double sumOfSqrDis = 0.0, sumOfWeights = 0.0;
  for (unsigned sId = 0; sId < sampleIds.size(); ++sId) {
    for (int i = 0; i < 3; ++i) {
      queryPt_[i] = cur_sample_poss[sId][i];
      const Vertex &vertex = (*movingSurf.GetVertexArray())[sampleIds[sId]];
      queryPt_[i+3] = colorWeight*vertex.color[i];
      queryPt_[i+6] = normalWeight*cur_sample_nors[sId][i];
    }
    kdTree_->annkSearch(
      queryPt_, 
      1, 
      nnIdx_, 
      dists_,
      0.0);

    Vector3f disVec = cur_sample_poss[sId] - fixed_poss_[nnIdx_[0]];

    (*corres)[sId].vertex_index = nnIdx_[0];
    double sqrPoint2PointDis = disVec.getSqrNorm();
    (*corres)[sId].weight =
      static_cast<float> (exp(-sqrPoint2PointDis/2/sigma2));
  }
}

void PWNonRigidAlign::SetFixedScan(const vector<Vertex> &fixed_points) {
  fixed_poss_.resize(fixed_points.size());
  fixed_nors_.resize(fixed_points.size());
  fixed_colors_.resize(fixed_points.size());
  for (unsigned pt_id = 0; pt_id < fixed_points.size(); ++pt_id) {
    fixed_poss_[pt_id] = fixed_points[pt_id].cur_pos;
    fixed_nors_[pt_id] = fixed_points[pt_id].cur_nor;
    fixed_colors_[pt_id] = fixed_points[pt_id].color;
  }
}
