#include "patch_generator.h"
#include "bounding_box.h"
#include "helper_functions.h"
#include "MgcEigen.h"
#include <algorithm>

void PatchGenerator::Compute(const TriangleMesh &mesh,
  const double &patch_radius,
  const double &suppert_size,
  PatchContainer *patch_container) {
  // Copy the vertex positions, the sampling is simply based on
  // vertex positions.
  vector<Vector3f> vertex_poss;
  vertex_poss.resize(mesh.GetVertexArray()->size());
  for (unsigned vertex_id = 0;
    vertex_id < mesh.GetVertexArray()->size();
    ++vertex_id)
    vertex_poss[vertex_id] = (*mesh.GetVertexArray())[vertex_id].ori_pos;

  // Generate patches
  ExtrinsicPatchGeneration(vertex_poss,
    patch_radius,
    suppert_size,
    &patch_container->surface_patches);

  for (unsigned patch_id = 0;
    patch_id < patch_container->surface_patches.size();
    ++patch_id) {
    InitializePatchPose(mesh,
      &patch_container->surface_patches[patch_id]);
  }
}

void PatchGenerator::ExtrinsicPatchGeneration(
  const vector<Vector3f> &point_positions,
  const double &patch_radius,
  const double &suppert_size,
  vector<SurfacePatch> *patches) {

  // Build a 3d-grid with resolution proportional to the target patch radius
  BoundingBox b_box;
  b_box.Initialize();
  for (unsigned point_id = 0; point_id < point_positions.size(); ++point_id)
    b_box.Insert_A_Point(point_positions[point_id]);

  double cell_width = 2*patch_radius;

  int dim_x = static_cast<int> (b_box.size[0]/cell_width) + 2;
  int dim_y = static_cast<int> (b_box.size[1]/cell_width) + 2;
  int dim_z = static_cast<int> (b_box.size[2]/cell_width) + 2;

  Vector3d lower_corner;
  lower_corner[0] = b_box.center_point[0] - dim_x*cell_width/2;
  lower_corner[1] = b_box.center_point[1] - dim_y*cell_width/2;
  lower_corner[2] = b_box.center_point[2] - dim_z*cell_width/2;

  // Allocate points in cells of this grid
  vector<vector<int>> cells;
  cells.resize(dim_x*dim_y*dim_z);

  for (unsigned point_id = 0; point_id < point_positions.size(); ++point_id) {
    const Vector3f &pos = point_positions[point_id];
    int cell_x = static_cast<int> ((pos[0] - lower_corner[0])/cell_width);
    int cell_y = static_cast<int> ((pos[1] - lower_corner[1])/cell_width);
    int cell_z = static_cast<int> ((pos[2] - lower_corner[2])/cell_width);

    int cell_id = (cell_x*dim_y + cell_y)*dim_z + cell_z;
    cells[cell_id].push_back(point_id);
  }

  // Generate a candidate sample from each cell that contains points
  vector<int> candidate_seed_ids;
  for (unsigned cell_id = 0; cell_id < cells.size(); ++cell_id) {
    const vector<int> &cell = cells[cell_id];
    if (cell.size() == 0)
      continue;

    // Find the bary-center of points in this cell
    Vector3f center;
    center[0] = center[1] = center[2] = 0.f;
    float sum = 0.f;
    for (unsigned i = 0; i < cell.size(); ++i) {
      center += point_positions[cell[i]];
      sum += 1.;
    }
    center /= sum;

    // Find the point that is closest to the bary-center
    float min_dis2 = 1e10f;
    int closest_point_id = -1;
    for (unsigned i = 0; i < cell.size(); ++i) {
      Vector3f dif = point_positions[cell[i]] - center;
      float dis2 = dif.getSqrNorm();
      if (dis2 < min_dis2) {
        min_dis2 = dis2;
        closest_point_id = cell[i];
      }
    }
    candidate_seed_ids.push_back(closest_point_id);
  }

  vector<Vector3f> candidate_seed_poss;
  candidate_seed_poss.resize(candidate_seed_ids.size());
  for (unsigned id = 0; id < candidate_seed_ids.size(); ++id)
    candidate_seed_poss[id] = point_positions[candidate_seed_ids[id]];

  // Perform Farthest point sampling in the Euclidean space.
  // At each iteration, the next sample is always picked as the one that
  // has the maximum distance to existing samples
  vector<int> patch_seed_ids;
  ExtrinsicFPS(candidate_seed_poss, patch_radius/2.f, &patch_seed_ids);

  for (unsigned patch_id = 0; patch_id < patch_seed_ids.size(); ++patch_id)
    patch_seed_ids[patch_id] = candidate_seed_ids[patch_seed_ids[patch_id]];

  // The support region of each patch
  int sub_grid_id = static_cast<int> (3.f*patch_radius/cell_width) + 1;
  patches->resize(patch_seed_ids.size());
  vector<vector<AssociWeight>> cand_patch_points;
  cand_patch_points.resize(patch_seed_ids.size());

  for (unsigned patch_id = 0; patch_id < patch_seed_ids.size(); ++patch_id) {
    int seed_id = patch_seed_ids[patch_id];
    Vector3f seed_pos = point_positions[seed_id];
    int seed_cell_x = static_cast<int> ((seed_pos[0] - lower_corner[0])/cell_width);
    int seed_cell_y = static_cast<int> ((seed_pos[1] - lower_corner[1])/cell_width);
    int seed_cell_z = static_cast<int> ((seed_pos[2] - lower_corner[2])/cell_width);

    // Perform range query search to obtain all nearest neighbors
    int window_x_left = max(0, seed_cell_x - sub_grid_id),
      window_x_right = min(dim_x - 1, seed_cell_x + sub_grid_id),
      window_y_left = max(0, seed_cell_y - sub_grid_id),
      window_y_right = min(dim_y - 1, seed_cell_y + sub_grid_id),
      window_z_left = max(0, seed_cell_z - sub_grid_id),
      window_z_right = min(dim_z - 1, seed_cell_z + sub_grid_id);

    SurfacePatch &patch = (*patches)[patch_id];
    patch.points.clear();
    patch.kernel_region_indices.clear();
    for (int cell_x = window_x_left; cell_x <= window_x_right; ++cell_x) {
      for (int cell_y = window_y_left; cell_y <= window_y_right; ++cell_y) {
        for (int cell_z = window_z_left; cell_z <= window_z_right; ++cell_z) {
          int cell_id = (cell_x*dim_y + cell_y)*dim_z + cell_z;
          const vector<int> &cell = cells[cell_id];
          if (cell.size() == 0)
            continue;

          for (unsigned i = 0; i < cell.size(); ++i) {
            int vertex_id = cell[i];
            Vector3f pos_dif = point_positions[vertex_id] - seed_pos;
            float dis2 = pos_dif.getSqrNorm();
            if (dis2 > 3.5f*3.5f*patch_radius*patch_radius)
              // Determines the support region
              continue;

            cand_patch_points[patch_id].push_back(AssociWeight(vertex_id, dis2));
          }
        }
      }
    }
  }

  // Determine the closest patch of each point
  vector<AssociWeight> point_patch_map;
  point_patch_map.resize(point_positions.size());
  for (unsigned pt_id = 0; pt_id < point_patch_map.size(); ++pt_id) {
    point_patch_map[pt_id].vertex_index = -1;
    point_patch_map[pt_id].weight = 1e10f;
  }

  for (unsigned patch_id = 0; patch_id < patches->size(); ++patch_id) {
    const vector<AssociWeight> &single_pts = cand_patch_points[patch_id];
    for (unsigned id = 0; id < single_pts.size(); ++id) {
      int vertex_id = single_pts[id].vertex_index;
      float weight = single_pts[id].weight;
      if (point_patch_map[vertex_id].weight > weight) {
        point_patch_map[vertex_id].weight = weight;
        point_patch_map[vertex_id].vertex_index = patch_id;
      }
    }
  }

  vector<float> patch_radii2;
  patch_radii2.resize(patches->size());
  for (unsigned patch_id = 0; patch_id < patches->size(); ++patch_id)
    patch_radii2[patch_id] = 0.f;

  float max_dis2 = 0.f;
  for (unsigned pt_id = 0; pt_id < point_patch_map.size(); ++pt_id) {
    int patch_id = point_patch_map[pt_id].vertex_index;
    patch_radii2[patch_id] = max(patch_radii2[patch_id],
      point_patch_map[pt_id].weight);
    max_dis2 = max(max_dis2, point_patch_map[pt_id].weight);
    (*patches)[patch_id].kernel_region_indices.push_back(pt_id);
  }

  // Re-compute each patch such that the radius of each patch is 1.5 times the maximum distance
  // from kernel points to patch centers
  max_dis2 *= 1.5f;
  printf("max_dis2 = %f.\n", max_dis2);
  for (unsigned patch_id = 0; patch_id < patches->size(); ++patch_id) {
    SurfacePatch &patch = (*patches)[patch_id];
    vector<AssociWeight> &single_pts = cand_patch_points[patch_id];
    unsigned num_remaining_point = 0;
    float tp = max(max_dis2,
      static_cast<float> (suppert_size*suppert_size*patch_radii2[patch_id]));
    for (unsigned i = 0; i < single_pts.size(); ++i) {
      if (single_pts[i].weight < tp) {
        single_pts[num_remaining_point].vertex_index = single_pts[i].vertex_index;
        float t = 1.f - sqrt(single_pts[i].weight/tp);
        single_pts[num_remaining_point].weight = t*t;
        num_remaining_point++;
      }
    }
    (*patches)[patch_id].points = single_pts;
  }
}


void PatchGenerator::InitializePatchPose(const TriangleMesh &mesh,
  SurfacePatch *patch) {
  // The implementation is exactly the same as 
  // Horn, B.K.P., 
  // ``Closed Form Solution of Absolute Orientation using Unit Quaternions,'' 
  // Journal of the Optical Society A, Vol. 4, No. 4, pp. 629--642, April 1987.

  if (patch->points.size() < 3) {
    patch->motion.Initialize();
    return;
  }
  Vector3f ori_center, cur_center;
  for (int i = 0; i < 3; ++i) {
    ori_center[i] = 0.f;
    cur_center[i] = 0.f;
  }
  float sum_of_weights = 0.f;
  for (unsigned i = 0; i < patch->points.size(); ++i) {
    const AssociWeight &aw = patch->points[i];
    ori_center += (*mesh.GetVertexArray())[aw.vertex_index].ori_pos*aw.weight;
    cur_center += (*mesh.GetVertexArray())[aw.vertex_index].cur_pos*aw.weight;
    sum_of_weights += aw.weight;
  }

  ori_center /= sum_of_weights;
  cur_center /= sum_of_weights;

  float matS[3][3];
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j)
      matS[i][j] = 0.f;

  for (unsigned i = 0; i < patch->points.size(); ++i) {
    const AssociWeight &aw = patch->points[i];
    Vector3f ori_vec = (*mesh.GetVertexArray())[aw.vertex_index].ori_pos - ori_center;
    Vector3f cur_vec = (*mesh.GetVertexArray())[aw.vertex_index].cur_pos - cur_center;
    for (int i = 0; i < 3; i++)
      for (int j = 0; j < 3; j++)
        matS[i][j] += ori_vec[i]*cur_vec[j];
  }   
  Mgc::Eigen *eigenSolver = new Mgc::Eigen(4);
  eigenSolver->Matrix(0,0) = matS[0][0] + matS[1][1] + matS[2][2];
  eigenSolver->Matrix(0,1) = matS[1][2] - matS[2][1];
  eigenSolver->Matrix(0,2) = matS[2][0] - matS[0][2];
  eigenSolver->Matrix(0,3) = matS[0][1] - matS[1][0];
  eigenSolver->Matrix(1,0) = eigenSolver->Matrix(0,1);
  eigenSolver->Matrix(1,1) = matS[0][0] - matS[1][1] - matS[2][2];
  eigenSolver->Matrix(1,2) = matS[0][1] + matS[1][0];
  eigenSolver->Matrix(1,3) = matS[0][2] + matS[2][0];
  eigenSolver->Matrix(2,0) = eigenSolver->Matrix(0,2);
  eigenSolver->Matrix(2,1) = eigenSolver->Matrix(1,2);
  eigenSolver->Matrix(2,2) = matS[1][1] - matS[0][0] - matS[2][2];
  eigenSolver->Matrix(2,3) = matS[1][2] + matS[2][1];
  eigenSolver->Matrix(3,0) = eigenSolver->Matrix(0,3);
  eigenSolver->Matrix(3,1) = eigenSolver->Matrix(1,3);
  eigenSolver->Matrix(3,2) = eigenSolver->Matrix(2,3);
  eigenSolver->Matrix(3,3) = matS[2][2] - matS[0][0] - matS[1][1];
  eigenSolver->DecrSortEigenStuff4();

  double Q[4];
  Q[0] = eigenSolver->GetEigenvector(0,0);
  Q[1] = eigenSolver->GetEigenvector(1,0);
  Q[2] = eigenSolver->GetEigenvector(2,0);
  Q[3] = eigenSolver->GetEigenvector(3,0);

  //  Matrix3d &rot = (*rotations)[v_id];

  patch->motion[1][0] = Q[0]*Q[0] + Q[1]*Q[1] - Q[2]*Q[2] - Q[3]*Q[3];
  patch->motion[2][1] = Q[0]*Q[0] + Q[2]*Q[2] - Q[1]*Q[1] - Q[3]*Q[3];
  patch->motion[3][2] = Q[0]*Q[0] + Q[3]*Q[3] - Q[1]*Q[1] - Q[2]*Q[2];
  patch->motion[2][0] = 2*(Q[1]*Q[2] - Q[0]*Q[3]);
  patch->motion[1][1] = 2*(Q[1]*Q[2] + Q[0]*Q[3]);
  patch->motion[3][0] = 2*(Q[1]*Q[3] + Q[0]*Q[2]);
  patch->motion[1][2] = 2*(Q[1]*Q[3] - Q[0]*Q[2]);
  patch->motion[3][1] = 2*(Q[2]*Q[3] - Q[0]*Q[1]);
  patch->motion[2][2] = 2*(Q[2]*Q[3] + Q[0]*Q[1]);
  patch->motion[0] = -(patch->motion[1]*ori_center[0]
  + patch->motion[2]*ori_center[1]
  + patch->motion[3]*ori_center[2]);
  patch->motion[0][0] += cur_center[0];
  patch->motion[0][1] += cur_center[1];
  patch->motion[0][2] += cur_center[2];
}
