#include "init_opt_struct.h"
#include <algorithm>
#include <set>
using namespace std;

void InitOptStruct::SmoothnessTerm(
  const TriangleMesh &mesh,
  const PatchContainer &patch_container,
  OptStructContainer *opt_struct) {
  int num_of_patches =
    static_cast<int> (patch_container.surface_patches.size());
  opt_struct->GetPatchQuadraticTerms()->resize(num_of_patches);

  vector<vector<int>> pointPatchMap;
  int num_of_points = static_cast<int> (mesh.GetVertexArray()->size());
  pointPatchMap.resize(num_of_points);
  for (unsigned patch_id = 0;
    patch_id < patch_container.surface_patches.size();
    ++patch_id) {
    const SurfacePatch &patch =
      patch_container.surface_patches[patch_id];
    for (unsigned i = 0; i < patch.points.size(); ++i) {
      pointPatchMap[patch.points[i].vertex_index].push_back(patch_id);
    }
  }

  vector<vector<AssociWeight>> patch_adj_graph;
  patch_adj_graph.resize(num_of_patches);
  for (int pt_id = 0; pt_id < num_of_points; ++pt_id) {
    const vector<int> &ids = pointPatchMap[pt_id];
    for (unsigned i = 0; i < ids.size(); ++i) {
      int patch1_id = ids[i];
      for (unsigned j = i+1; j < ids.size(); ++j) {
        int patch2_id = ids[j];
        int id = -1;
        for (unsigned k = 0; k < patch_adj_graph[patch1_id].size(); ++k) {
          if (patch_adj_graph[patch1_id][k].vertex_index == patch2_id) {
            id = k;
            break;
          }
        }
        if (id >= 0)
          patch_adj_graph[patch1_id][id].weight += 1.f;
        else
          patch_adj_graph[patch1_id].push_back(AssociWeight(patch2_id, 1.f));

        id = -1;
        for (unsigned k = 0; k < patch_adj_graph[patch2_id].size(); ++k) {
          if (patch_adj_graph[patch2_id][k].vertex_index == patch1_id) {
            id = k;
            break;
          }
        }
        if (id >= 0)
          patch_adj_graph[patch2_id][id].weight += 1.f;
        else
          patch_adj_graph[patch2_id].push_back(AssociWeight(patch1_id, 1.f));
      }
    }
  }

  unsigned knn = 6;
  vector<set<int>> buf_neigh_patches;
  buf_neigh_patches.resize(num_of_patches);
  for (int patch1_id = 0; patch1_id < num_of_patches; ++patch1_id) {
    sort(patch_adj_graph[patch1_id].begin(), patch_adj_graph[patch1_id].end());
    if (patch_adj_graph[patch1_id].size() > knn)
      patch_adj_graph[patch1_id].resize(knn);
    for (unsigned k = 0; k < patch_adj_graph[patch1_id].size(); ++k) {
      int patch2_id = patch_adj_graph[patch1_id][k].vertex_index;
      buf_neigh_patches[patch1_id].insert(patch2_id);
      buf_neigh_patches[patch2_id].insert(patch1_id);
    }
  }


  vector<Matrix4d> *diag_terms =
    opt_struct->GetReguDiagonalTerms();
  diag_terms->resize(num_of_patches);
  for (int patch_id = 0; patch_id < num_of_patches; ++patch_id) {
    Matrix4d &mat = (*opt_struct->GetReguDiagonalTerms())[patch_id];
    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        mat[i][j] = 0.0;
  }

  vector<float> point_weights;
  point_weights.resize(num_of_points);
  for (int pt_id = 0; pt_id < num_of_points; ++pt_id)
    point_weights[pt_id] = 0.f;

  vector<NonDiagonalTerm> *nondiag_terms =
    opt_struct->GetReguNonDiagonalTerms();
  nondiag_terms->clear();
  for (int patch_id = 0; patch_id < num_of_patches; ++patch_id) {
    for (set<int>::iterator iter_int = buf_neigh_patches[patch_id].begin();
      iter_int != buf_neigh_patches[patch_id].end();
      ++iter_int) {
      if (patch_id >= *iter_int)
        continue;

      const SurfacePatch &patch1 =
        patch_container.surface_patches[patch_id];
      const SurfacePatch &patch2 = 
        patch_container.surface_patches[*iter_int];

      for (unsigned i = 0; i < patch1.points.size(); ++i)
        point_weights[patch1.points[i].vertex_index] =
        -patch1.points[i].weight;

      for (unsigned i = 0; i < patch2.points.size(); ++i) {
        if (point_weights[patch2.points[i].vertex_index] < 0.f) {
          point_weights[patch2.points[i].vertex_index] *=
            - patch2.points[i].weight;
        }
      }

      Matrix4d matA;
      for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
          matA[i][j] = 0.f;

      for (unsigned k = 0; k < patch1.points.size(); ++k) {
        int pt_id = patch1.points[k].vertex_index;
        if (point_weights[pt_id] > 0.f) {
          const Vertex &vertex = (*mesh.GetVertexArray())[pt_id];
          float vec[4];
          vec[0] = 1.f;
          vec[1] = vertex.ori_pos[0];
          vec[2] = vertex.ori_pos[1];
          vec[3] = vertex.ori_pos[2];
          for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
              matA[i][j] += point_weights[pt_id]*vec[i]*vec[j];
        }
      }

      for (unsigned i = 0; i < patch1.points.size(); ++i)
        point_weights[patch1.points[i].vertex_index] = 0.f;
      for (unsigned i = 0; i < patch2.points.size(); ++i)
        point_weights[patch2.points[i].vertex_index] = 0.f;

      (*diag_terms)[patch_id] += matA;
      (*diag_terms)[*iter_int] += matA;
      NonDiagonalTerm term1, term2;
      term1.row_id = patch_id;
      term1.col_id = *iter_int;
      term1.matA = -matA;
      term2.row_id = *iter_int;
      term2.col_id = patch_id;
      term2.matA = -matA;
      nondiag_terms->push_back(term1);
      nondiag_terms->push_back(term2);
    }
  }
  printf("    num_patches = %d, num_edges = %d\n", diag_terms->size(), nondiag_terms->size()/2);
}

void InitOptStruct::DataTerm(
  const ShapeContainer &moving_scan,
  const PatchContainer &deform_struct,
  const PointCloud &fixed_surf,
  const vector<int> &foot_point_indices,
  const double &weightPointPlaneDis,
  OptStructContainer *opt_struct) {
  if (foot_point_indices.size() != moving_scan.surface.GetVertexArray()->size())
    return;

  for (unsigned patch_id = 0;
    patch_id < deform_struct.surface_patches.size();
    ++patch_id) {
    const SurfacePatch &patch = deform_struct.surface_patches[patch_id];
    PatchQuadraticTerm &patch_data_term =
      (*opt_struct->GetPatchQuadraticTerms())[patch_id];
    patch_data_term.SetZero();

    for (unsigned i = 0; i < patch.kernel_region_indices.size(); ++i) {
      int vertex_index = patch.kernel_region_indices[i];
      const Vertex &vertex =
        (*moving_scan.surface.GetVertexArray())[vertex_index];
      const Vector3f &pos = vertex.ori_pos;
      int foot_point_id = foot_point_indices[vertex_index];
      if (foot_point_id == -1)
        continue;

      const Vertex &foot_point =
        (*fixed_surf.GetVertexArray())[foot_point_id];
      const Vector3f &foot_pos = foot_point.ori_pos;
      const Vector3f &foot_nor = foot_point.ori_nor;

      double jacobi[12], d; //store the jacobi matrix;
      d = foot_nor*foot_pos;
      for (int k = 0; k < 3; ++k) {
        jacobi[k] = foot_nor[k];
        jacobi[k+3] = foot_nor[k]*pos[0];
        jacobi[k+6] = foot_nor[k]*pos[1];
        jacobi[k+9] = foot_nor[k]*pos[2];
      }
      for (int j = 0; j < 12; ++j) {
        for (int k = 0; k < 12; ++k)
          patch_data_term.matA[j][k] +=
          jacobi[j]*jacobi[k]*weightPointPlaneDis;
        patch_data_term.vecb[j] += jacobi[j]*d*weightPointPlaneDis;
      }
      patch_data_term.c += d*d*weightPointPlaneDis;

      jacobi[0] = 1.0;
      jacobi[1] = pos[0];
      jacobi[2] = pos[1];
      jacobi[3] = pos[2];
      double point_point_weight = 1 - weightPointPlaneDis;
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
          for (int k = 0; k < 3; ++k)
            patch_data_term.matA[3*i+k][3*j+k] +=
            point_point_weight*jacobi[i]*jacobi[j];

        for (int k = 0; k < 3; ++k)
          patch_data_term.vecb[3*i+k] +=
          point_point_weight*jacobi[i]*foot_pos[k];

      }
      patch_data_term.c += point_point_weight*(foot_pos.getSqrNorm());
    }
    // Ensure that all the patches are non-degenerate
    double lambda = 1e-2;
    for (int i = 0; i < 12; ++i)
      patch_data_term.matA[i][i] += lambda;

    const Affine3d &aff = patch.motion;
    for (int i = 0; i < 4; ++i)
      for (int k = 0; k < 3; ++k)
        patch_data_term.vecb[3*i + k] += aff[i][k]*lambda;

  }
}

void InitOptStruct::DataTerm(const ShapeContainer &moving_scan,
  const PatchContainer &deform_struct,
  const vector<int> &sample_indices,
  const vector<Vector3f> &fixed_poss,
  const vector<Vector3f> &fixed_nors,
  const vector<AssociWeight> &corres,
  const double &weightPointPlaneDis,
  OptStructContainer *opt_struct) {
  vector<int> vertex_sIds;
  vertex_sIds.resize(moving_scan.surface.GetVertexArray()->size());
  for (unsigned v_id = 0; v_id < vertex_sIds.size(); ++v_id)
    vertex_sIds[v_id] = -1;

  for (unsigned i = 0; i < sample_indices.size(); ++i)
    vertex_sIds[sample_indices[i]] = i;

  for (unsigned patch_id = 0;
    patch_id < deform_struct.surface_patches.size();
    ++patch_id) {
    const SurfacePatch &patch = deform_struct.surface_patches[patch_id];
    PatchQuadraticTerm &patch_data_term =
      (*opt_struct->GetPatchQuadraticTerms())[patch_id];
    patch_data_term.SetZero();

    for (unsigned i = 0; i < patch.kernel_region_indices.size(); ++i) {
      int vertex_index = patch.kernel_region_indices[i];
      int sample_index = vertex_sIds[vertex_index];
      if (sample_index == -1)
        continue;

      const Vertex &vertex =
        (*moving_scan.surface.GetVertexArray())[vertex_index];
        
      const Vector3f &pos = vertex.ori_pos;
      int foot_point_id = corres[sample_index].vertex_index;
  
      const Vector3f &foot_pos = fixed_poss[foot_point_id];
      const Vector3f &foot_nor = fixed_nors[foot_point_id];

      double weightPlane = weightPointPlaneDis*corres[sample_index].weight;
      double weightPoint = (1.0-weightPointPlaneDis)*corres[sample_index].weight;
      double jacobi[12], d; //store the jacobi matrix;
      d = foot_nor*foot_pos;
      for (int k = 0; k < 3; ++k) {
        jacobi[k] = foot_nor[k];
        jacobi[k+3] = foot_nor[k]*pos[0];
        jacobi[k+6] = foot_nor[k]*pos[1];
        jacobi[k+9] = foot_nor[k]*pos[2];
      }
      for (int j = 0; j < 12; ++j) {
        for (int k = 0; k < 12; ++k)
          patch_data_term.matA[j][k] +=
          jacobi[j]*jacobi[k]*weightPlane;
        patch_data_term.vecb[j] += jacobi[j]*d*weightPlane;
      }
      patch_data_term.c += d*d*weightPlane;

      jacobi[0] = 1.0;
      jacobi[1] = pos[0];
      jacobi[2] = pos[1];
      jacobi[3] = pos[2];
      for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j)
          for (int k = 0; k < 3; ++k)
            patch_data_term.matA[3*i+k][3*j+k] +=
            weightPoint*jacobi[i]*jacobi[j];

        for (int k = 0; k < 3; ++k)
          patch_data_term.vecb[3*i+k] +=
          weightPoint*jacobi[i]*foot_pos[k];

      }
      patch_data_term.c += weightPoint*(foot_pos.getSqrNorm());
    }
    // Ensure that all the patches are non-degenerate
    double lambda = 1e-2;
    for (int i = 0; i < 12; ++i)
      patch_data_term.matA[i][i] += lambda;

    const Affine3d &aff = patch.motion;
    for (int i = 0; i < 4; ++i)
      for (int k = 0; k < 3; ++k)
        patch_data_term.vecb[3*i + k] += aff[i][k]*lambda;

  }  
}