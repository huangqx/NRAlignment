#include "target_point_generator.h"
#include "helper_functions.h"
#include "bounding_box.h"
#include <algorithm>
#include <set>
using namespace std;


void TargetPointGenerator::Compute(const vector<ShapeContainer> &input_scans,
  const RefSurfPara &para,
  vector<Vertex> *foot_points,
  vector<vector<int>> *foot_point_indices) {
  printf("starting octree.\n");
  // Generate the octree
  Octree3D* octree = GenerateOctree(input_scans, para.gridRes);
  if (octree == NULL)
    return;

  // Initialize the target point arrays
  foot_point_indices->resize(input_scans.size());
  for (unsigned scan_id = 0; scan_id < foot_point_indices->size(); ++scan_id) {
    const ShapeContainer &scan = input_scans[scan_id];
    (*foot_point_indices)[scan_id].resize(scan.surface.GetVertexArray()->size());
    for (unsigned point_id = 0;
      point_id < (*foot_point_indices)[scan_id].size();
      ++point_id) {
      (*foot_point_indices)[scan_id][point_id] = -1;
    }
  }
  
  printf("starting octree 1.\n");
  // Obtain all leaf nodes
  vector<int> leaf_cell_x_ids, leaf_cell_y_ids, leaf_cell_z_ids;
  octree->CollectAllLeafs(&leaf_cell_x_ids, &leaf_cell_y_ids, &leaf_cell_z_ids);
  
  int max_cell_coord = (1<<octree->max_depth)-1;

  foot_points->clear();
  
  // For each leaf cell, perform clustering in its neighbors
  for (unsigned leaf_id = 0; leaf_id < leaf_cell_x_ids.size(); ++leaf_id) {
    
    // Collect all data points that are in the neighboring cells
    // of this leaf cell
    int window_x_left = max(0, leaf_cell_x_ids[leaf_id]-para.halfWindowWidth),
      window_x_right = min(max_cell_coord, leaf_cell_x_ids[leaf_id]+para.halfWindowWidth),
      window_y_left = max(0, leaf_cell_y_ids[leaf_id]-para.halfWindowWidth),
      window_y_right = min(max_cell_coord, leaf_cell_y_ids[leaf_id]+para.halfWindowWidth),
      window_z_left = max(0, leaf_cell_z_ids[leaf_id]-para.halfWindowWidth),
      window_z_right = min(max_cell_coord, leaf_cell_z_ids[leaf_id]+para.halfWindowWidth);

    // For efficient memory organization, obtain the total number of points
    unsigned num_of_total_points = 0;
    for (int x = window_x_left; x <= window_x_right; ++x) {
      for (int y = window_y_left; y <= window_y_right; ++y) {
        for (int z = window_z_left; z <= window_z_right; ++z) {
          const OctreeLeaf3D *neigh_leaf = octree->QueryLeaf(x, y, z);
          if (neigh_leaf == NULL)
            continue;

          num_of_total_points += neigh_leaf->point_indices.size()/2;
        }
      }
    }

    //  Collect all data points in the moving window centered at this cell
    vector<int> scan_ids, vertex_ids;
    vector<Vector3f> vertex_poss, vertex_nors, vertex_colors;
    vector<bool> vertex_center_flags;

    scan_ids.resize(num_of_total_points);
    vertex_ids.resize(num_of_total_points);
    vertex_poss.resize(num_of_total_points);
    vertex_nors.resize(num_of_total_points);
    vertex_colors.resize(num_of_total_points);
    vertex_center_flags.resize(num_of_total_points);

    unsigned point_id = 0;
    for (int x = window_x_left; x <= window_x_right; ++x) {
      for (int y = window_y_left; y <= window_y_right; ++y) {
        for (int z = window_z_left; z <= window_z_right; ++z) {
          const OctreeLeaf3D *neigh_leaf = octree->QueryLeaf(x, y, z);
          if (neigh_leaf == NULL)
            continue;

          bool is_center = (x == leaf_cell_x_ids[leaf_id])
            && (y == leaf_cell_y_ids[leaf_id])
            && (z == leaf_cell_z_ids[leaf_id]);
       
          for (unsigned i = 0; i < neigh_leaf->point_indices.size()/2; ++i) {
            int scan_id = neigh_leaf->point_indices[2*i];
            int vertex_id = neigh_leaf->point_indices[2*i+1];
            const TriangleMesh &scan = input_scans[scan_id].surface;
            const Vertex &vertex = (*scan.GetVertexArray())[vertex_id];

            // push point
            scan_ids[point_id] = scan_id;
            vertex_ids[point_id] = vertex_id;
            vertex_poss[point_id] = vertex.cur_pos;
            vertex_nors[point_id] = vertex.cur_nor;
            vertex_colors[point_id] = vertex.color;
            vertex_center_flags[point_id] = is_center;
            point_id++;
          }
        }
      }
    }
    // determine point weights. Points are weighted
    // based on their distances to the center of this leaf cell
    vector<float> vertex_weights;
    GaussianWeighting(vertex_poss,
      vertex_center_flags,
      para.gridRes,
      &vertex_weights);

    // For efficiency, remove points with small weights
    int remaining_point_id = 0;
    for (unsigned point_id = 0; point_id < num_of_total_points; ++point_id) {
      if (vertex_weights[point_id] > 1e-1f || vertex_center_flags[point_id]) {
        vertex_weights[remaining_point_id] = vertex_weights[point_id];
        scan_ids[remaining_point_id] = scan_ids[point_id];
        vertex_ids[remaining_point_id] = vertex_ids[point_id];
        vertex_poss[remaining_point_id] = vertex_poss[point_id];
        vertex_nors[remaining_point_id] = vertex_nors[point_id];
        vertex_colors[remaining_point_id] = vertex_colors[point_id];
        vertex_center_flags[remaining_point_id] = vertex_center_flags[point_id];
        remaining_point_id++;
      }
    }
    vertex_weights.resize(remaining_point_id);
    scan_ids.resize(remaining_point_id);
    vertex_ids.resize(remaining_point_id);
    vertex_poss.resize(remaining_point_id);
    vertex_nors.resize(remaining_point_id);
    vertex_colors.resize(remaining_point_id);
    vertex_center_flags.resize(remaining_point_id);

    // perform farthest point based clustering
    vector<vector<int>> clusters;
    FPSClustering(vertex_nors,
      vertex_colors,
      para.weightColor,
      para.clusterSize,
      &clusters);

    for (unsigned cluster_id = 0; cluster_id < clusters.size(); ++cluster_id) {
      const vector<int> &cluster_point_ids = clusters[cluster_id];
      int min_scan_id = 10000, max_scan_id = 0;
      for (unsigned i = 0; i < cluster_point_ids.size(); ++i) {
        int point_id = cluster_point_ids[i];
        min_scan_id = min(min_scan_id, scan_ids[point_id]);
        max_scan_id = max(max_scan_id, scan_ids[point_id]);
      }

      // A target point makes sense if this cell contains points from
      // different scans
      if (max_scan_id == min_scan_id)
        continue;

      // Generate a target point
      Vertex new_ft;
      for (unsigned i = 0; i < cluster_point_ids.size(); ++i) {
        int point_id = cluster_point_ids[i];
        new_ft.ori_pos += vertex_poss[point_id]*vertex_weights[point_id];
        new_ft.ori_nor += vertex_nors[point_id]*vertex_weights[point_id];
        new_ft.color += vertex_colors[point_id]*vertex_weights[point_id];
        new_ft.density += vertex_weights[point_id];
        if (vertex_center_flags[point_id]) {
          (*foot_point_indices)[scan_ids[point_id]][vertex_ids[point_id]] =
            static_cast<int> (foot_points->size());
        }
      }
      new_ft.ori_pos /= new_ft.density;
      new_ft.ori_nor /= new_ft.density;
      new_ft.ori_nor /= sqrt(new_ft.ori_nor.getSqrNorm());
      new_ft.color /= new_ft.density;
      new_ft.cur_pos = new_ft.ori_pos;
      new_ft.cur_nor = new_ft.ori_nor;
      foot_points->push_back(new_ft);
    }
  }
  // release octree
  delete octree;
  printf("    The reference surface has %d foot points.\n", foot_points->size());
}

Octree3D* TargetPointGenerator::GenerateOctree(
  const vector<ShapeContainer> &input_scans,
  const double &grid_size) {
  // Generate the bounding box
  BoundingBox b_box;
  b_box.Initialize();
  for (unsigned scan_id = 0; scan_id < input_scans.size(); ++scan_id) {
    const TriangleMesh &scan = input_scans[scan_id].surface;
    for (unsigned vertex_id = 0;
      vertex_id < scan.GetVertexArray()->size();
      ++vertex_id) {
      const Vertex &vertex = (*scan.GetVertexArray())[vertex_id];
      b_box.Insert_A_Point(vertex.cur_pos);
    }
  }
  
  double cube_size = max(max(b_box.size[0], b_box.size[1]), b_box.size[2]);
  if (cube_size < grid_size*2.0)
    return NULL;

  max_depth_ = static_cast<int> (int(log((cube_size/grid_size)+1.0)/log(2.0))) + 1;
  cube_size = grid_size*(1<<max_depth_);
  Octree3D *octree = new Octree3D();
  octree->max_depth = max_depth_;
  octree->grid_size = static_cast<float> (grid_size);
  octree->left_corner[0] = static_cast<float> (b_box.center_point[0] - (cube_size/2.0));
  octree->left_corner[1] = static_cast<float> (b_box.center_point[1] - (cube_size/2.0));
  octree->left_corner[2] = static_cast<float> (b_box.center_point[2] - (cube_size/2.0));
  octree->root = NULL;

  for (unsigned scan_id = 0; scan_id < input_scans.size(); ++scan_id) {
    const TriangleMesh &scan = input_scans[scan_id].surface;
    for (unsigned vertex_id = 0;
      vertex_id < scan.GetVertexArray()->size();
      ++vertex_id) {
      const Vector3f &pos = (*scan.GetVertexArray())[vertex_id].cur_pos;
      cell_offsets_[0] = static_cast<int> ((pos[0] - octree->left_corner[0])/grid_size);
      cell_offsets_[1] = static_cast<int> ((pos[1] - octree->left_corner[1])/grid_size);
      cell_offsets_[2] = static_cast<int> ((pos[2] - octree->left_corner[2])/grid_size);
      depth_ = 0;
      InsertAPoint(scan_id, vertex_id, &octree->root);
    }
  }
  return octree;
}

void TargetPointGenerator::InsertAPoint(const int &scan_index,
  const int &point_index,
  Node3D** current_node) {
  if (depth_ < max_depth_) {
    if (*current_node == NULL)
      *current_node = new OctreeNode3D();

    OctreeNode3D *nl_node = (OctreeNode3D*)(*current_node);
    int half_box_width = 1<<(max_depth_ - depth_ - 1);
    int i = 0, j = 0, k = 0;
    if (cell_offsets_[0] >= half_box_width) {
      cell_offsets_[0] -= half_box_width;
      i = 1;
    }
    if (cell_offsets_[1] >= half_box_width) {
      cell_offsets_[1] -= half_box_width;
      j = 1;
    }
    if (cell_offsets_[2] >= half_box_width) {
      cell_offsets_[2] -= half_box_width;
      k = 1;
    }
    int child_offset = (i<<2) + (j<<1) + k;
    depth_++;
    InsertAPoint(scan_index, point_index, &(nl_node->children[child_offset]));
  } else {
    if (*current_node == NULL) 
      *current_node = new OctreeLeaf3D();
    
    OctreeLeaf3D *ol_node = (OctreeLeaf3D*)(*current_node);
    ol_node->point_indices.push_back(scan_index);
    ol_node->point_indices.push_back(point_index);
  }
}

/*
void OctreeNode3D::FindTargetPoints(
  const vector<int> &point_flags,
  const RefSurfPara &para,
  vector<Vector3f> *point_positions,
  vector<Vector3f> *point_normals,
  vector<float> *point_weights,
  vector<Vertex> *foot_points) {
  for (int i = 0; i < 8; ++i) {
    if (children[i] != NULL)
      children[i]->FindTargetPoints(
      point_flags,
      para,
      point_positions,
      point_normals,
      point_weights,
      foot_points);
  }
}

// A helper function which performs surfel fitting
void SurfelFitting(const vector<Vector3f> &point_positions,
  const vector<Vector3f> &point_normals,
  const vector<float> &point_weights,
  const vector<int> &indices,
  Vector3f *pos,
  Vector3f *nor) {
  if (1) {
    float sum_of_weights = 0.f;
    for (int k = 0; k < 3; ++k) {
      (*pos)[k] = 0.f;
      (*nor)[k] = 0.f;
    }
    for (unsigned i = 0; i < indices.size(); ++i) {
      *pos += point_positions[indices[i]]*point_weights[indices[i]];
      *nor += point_normals[indices[i]]*point_weights[indices[i]];
      sum_of_weights += point_weights[indices[i]];
    }
    if (sum_of_weights < 1.f) {
      int hqx = 10;
    }
    *pos /= sum_of_weights;
    *nor /= sum_of_weights;
  } else {
    Moment3d moment;
    moment.SetZero();
    Vector3f normal;
    normal[0] = normal[1] = normal[2] = 0.f;
    for (unsigned i = 0; i < indices.size(); ++i) {
      moment.InsertAPoint(point_positions[indices[i]], point_weights[indices[i]]);
      normal += point_normals[indices[i]]*point_weights[indices[i]];
    }
    normal.normalize();
    for (int i = 0;i < 3; ++i)
      (*pos)[i] = static_cast<float> (moment.order1[i]/moment.order0);
    
    Mgc::Eigen *eigen_solver = new Mgc::Eigen(3);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        eigen_solver->Matrix(i, j) = moment.order2[i][j]
        - moment.order1[i]*moment.order1[j]/moment.order0;
      }
    }
    eigen_solver->IncrSortEigenStuff3();
    float max_inner = -1.f;
    for (int i = 0; i < 3; ++i) {
      Vector3f candidate_normal;
      candidate_normal[0] = static_cast<float> (eigen_solver->Matrix(0, i));
      candidate_normal[1] = static_cast<float> (eigen_solver->Matrix(1, i));
      candidate_normal[2] = static_cast<float> (eigen_solver->Matrix(2, i));
      float inner = candidate_normal*normal;
      if (fabs(inner) > max_inner) {
        max_inner = fabs(inner);
        if (inner > 0)
          *nor = candidate_normal;
        else
          *nor = -candidate_normal;
      }
    }
    delete eigen_solver;
  }
}

// A helper function which clustering points based on normal information
void Clustering(const vector<Vector3f> &point_normals,
  const vector<float> &point_weights,
  const vector<int> &indices,
  const float &min_inner,
  vector<vector<int>> *clusters) {
  vector<Vector3f> cluster_nor;
  vector<float> cluster_weights;
  clusters->clear();
  for (unsigned i = 0; i < indices.size(); ++i) {
    int pt_id = indices[i];
    const Vector3f &nor = point_normals[pt_id];
    const float &weight = point_weights[pt_id];
    float max_inner = -1.f;
    int cluster_id = -1;
    for (unsigned k = 0; k < clusters->size(); ++k) {
      float inner = nor*cluster_nor[k]/cluster_weights[k];
      if (inner > max_inner) {
        max_inner = inner;
        cluster_id = k;
      }
    }
    if (max_inner > min_inner) {
      (*clusters)[cluster_id].push_back(pt_id);
      cluster_nor[cluster_id] += nor*weight;
      cluster_weights[cluster_id] += weight;
    } else {
      vector<int> new_cluster;
      new_cluster.resize(1);
      new_cluster[0] = pt_id;
      clusters->push_back(new_cluster);
      cluster_nor.push_back(nor*weight);
      cluster_weights.push_back(weight);
    }
  }
}

void OctreeLeaf3D::FindTargetPoints(
  const vector<int> &point_flags,
  const RefSurfPara &para,
  vector<Vector3f> *point_positions,
  vector<Vector3f> *point_normals,
  vector<float> *point_weights,
  vector<Vertex> *foot_points) {
  vector<vector<int>> clusters;
  Clustering(*point_normals,
    *point_weights,
    point_indices,
    para.min_inner,
    &clusters);

  for (unsigned id = 0; id < clusters.size(); ++id) {
    const vector<int> &cluster = clusters[id];
    Vertex new_vertex;
    SurfelFitting(*point_positions,
      *point_normals,
      *point_weights,
      cluster,
      &new_vertex.ori_pos,
      &new_vertex.ori_nor);
    int min_scan_id = 100000, max_scan_id = 0;
    for (unsigned i = 0; i < cluster.size(); ++i) {
      min_scan_id = min(min_scan_id, point_flags[cluster[i]]);
      max_scan_id = max(max_scan_id, point_flags[cluster[i]]);
    }
    for (unsigned i = 0; i < cluster.size(); ++i) {
      (*point_positions)[cluster[i]] = new_vertex.ori_pos;
      (*point_normals)[cluster[i]] = new_vertex.ori_nor;
    }
    if (min_scan_id == max_scan_id) {
      for (unsigned i = 0; i < cluster.size(); ++i) {
        (*point_weights)[cluster[i]] = 0.000001f;
      }
    } else {
    }
    foot_points->push_back(new_vertex);
  }
}*/