#include "helper_functions.h"

void GaussianWeighting(const vector<Vector3f> &point_positions,
  const vector<bool> &is_center_flags,
  const double &sigma,
  vector<float> *point_weights) {
  Vector3f center;
  center[0] = center[1] = center[2] = 0.f;
  float sum = 0.f;
  for (unsigned pt_id = 0; pt_id < point_positions.size(); ++pt_id) {
    if (is_center_flags[pt_id]) {
      center += point_positions[pt_id];
      sum += 1.f;
    }
  }
  center /= sum;

  int closest_point_id = -1;
  float min_dis2 = 1e10f;
  for (unsigned pt_id = 0; pt_id < point_positions.size(); ++pt_id) {
    if (is_center_flags[pt_id]) {
      Vector3f dif = point_positions[pt_id] - center;
      float dis2 = dif.getSqrNorm();
      if (dis2 < min_dis2) {
        min_dis2 = dis2;
        closest_point_id = pt_id;
      }
    }
  }

  center = point_positions[closest_point_id];
  point_weights->resize(point_positions.size());

  float sigma2 = static_cast<float>(sigma*sigma);
  for (unsigned point_id = 0; point_id < point_positions.size(); ++point_id) {
    Vector3f dif = point_positions[point_id] - center;
    (*point_weights)[point_id] = exp(-dif.getSqrNorm()*2.f/sigma2);
  }
}

void FPSClustering(const vector<Vector3f> &point_normals,
  const vector<Vector3f> &point_colors,
  const double &weight_color,
  const double &cluster_size,
  vector<vector<int>> *clusters) {
  if (point_normals.size() == 1) {
    vector<int> cluster;
	cluster.resize(1);
	cluster[0] = 1;
	clusters->push_back(cluster);
	return;
  }
  // Store the point cluster map
  vector<double> point_cluster_dis;
  vector<int> point_cluster_ids;
  point_cluster_dis.resize(point_normals.size());
  point_cluster_ids.resize(point_normals.size());
  for (unsigned pt_id = 0; pt_id < point_normals.size(); ++pt_id) {
    point_cluster_ids[pt_id] = -1;
    point_cluster_dis[pt_id] = 1e10f;
  }

  // Find the first seed point
  double max_dis2 = 0.f;
  int random_point_id = 0, seed_id = -1;
  for (unsigned pt_id = 0; pt_id < point_normals.size(); ++pt_id) {
    Vector3f nor_dif = point_normals[pt_id] - point_normals[random_point_id];
    Vector3f color_dif = point_colors[pt_id] - point_colors[random_point_id];
    double dis2 = nor_dif.getSqrNorm() + weight_color*color_dif.getSqrNorm();
    if (dis2 > max_dis2) {
      max_dis2 = dis2;
      seed_id = pt_id;
    }
  }

  int num_of_clusters = 0;
  while (true) {
    num_of_clusters++;
    max_dis2 = 0.f;
    int next_seed_id = -1;
    for (unsigned pt_id = 0; pt_id < point_normals.size(); ++pt_id) {
      Vector3f nor_dif = point_normals[pt_id] - point_normals[seed_id];
      Vector3f color_dif = point_colors[pt_id] - point_colors[seed_id];
      double dis2 = nor_dif.getSqrNorm() + weight_color*color_dif.getSqrNorm();
      if (dis2 < point_cluster_dis[pt_id]) {
        point_cluster_dis[pt_id] = dis2;
        point_cluster_ids[pt_id] = num_of_clusters-1;
      }
      if (point_cluster_dis[pt_id] > max_dis2) {
        max_dis2 = point_cluster_dis[pt_id];
        next_seed_id = pt_id;
      }
    }
    if (max_dis2 < cluster_size*cluster_size)
      break;
    seed_id = next_seed_id;
  }
  clusters->resize(num_of_clusters);
  for (unsigned pt_id = 0; pt_id < point_normals.size(); ++pt_id)
    (*clusters)[point_cluster_ids[pt_id]].push_back(pt_id);
}

void ExtrinsicFPS(const vector<Vector3f> &point_poss,
  const double &sampling_density,
  vector<int> *sample_indices) {
  // Store the point cluster map
  vector<float> closest_dis;
  closest_dis.resize(point_poss.size());
  for (unsigned pt_id = 0; pt_id < point_poss.size(); ++pt_id)
    closest_dis[pt_id] = 1e10f;

  // Find the first seed point
  float max_dis2 = 0.f;
  int random_point_id = 0, seed_id = -1;
  for (unsigned pt_id = 0; pt_id < point_poss.size(); ++pt_id) {
    Vector3f pos_dif = point_poss[pt_id] - point_poss[random_point_id];
    float dis2 = pos_dif.getSqrNorm();
    if (dis2 > max_dis2) {
      max_dis2 = dis2;
      seed_id = pt_id;
    }
  }

  while (true) {
    sample_indices->push_back(seed_id);
    max_dis2 = 0.f;
    int next_seed_id = -1;
    for (unsigned pt_id = 0; pt_id < point_poss.size(); ++pt_id) {
      Vector3f pos_dif = point_poss[pt_id] - point_poss[seed_id];
      float dis2 = pos_dif.getSqrNorm();
      if (dis2 < closest_dis[pt_id])
        closest_dis[pt_id] = dis2;

      if (closest_dis[pt_id] > max_dis2) {
        max_dis2 = closest_dis[pt_id];
        next_seed_id = pt_id;
      }
    }
    if (max_dis2 < sampling_density*sampling_density)
      break;
    seed_id = next_seed_id;
  }  
}