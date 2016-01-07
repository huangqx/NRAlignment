#include <algorithm>
#include <vector>
#include <set>
using namespace std;

#include "triangle_mesh.h"
#include "io_helper.h"

TriangleMesh::TriangleMesh() {
}

TriangleMesh::~TriangleMesh() {
}

void TriangleMesh::Clear() {
  vertices_.clear();
  faces_.clear();
  texture_coords_.clear();
  quad_faces_.clear();
}

void TriangleMesh::OriginalFaceNormal(int face_id, Vector3f *nor) const {
  const Face &face = faces_[face_id];

  const Vertex &v0 = vertices_[face.vertex_indices[0]];
  const Vertex &v1 = vertices_[face.vertex_indices[1]];
  const Vertex &v2 = vertices_[face.vertex_indices[2]];

  Vector3f p21 = v2.ori_pos - v1.ori_pos;
  Vector3f p10 = v1.ori_pos - v0.ori_pos;

  *nor = p10.crossProduct(p21);
  *nor /= sqrt(nor->getSqrNorm());

  float face_area = TriangleArea(
    v0.cur_pos,
    v1.cur_pos,
    v2.cur_pos);

  *nor *=face_area;
}

void TriangleMesh::CurrentFaceNormal(int face_id, Vector3f *nor) const {
  const Face &face = faces_[face_id];

  const Vertex &v0 = vertices_[face.vertex_indices[0]];
  const Vertex &v1 = vertices_[face.vertex_indices[1]];
  const Vertex &v2 = vertices_[face.vertex_indices[2]];

  Vector3f p21 = v2.cur_pos - v1.cur_pos;
  Vector3f p10 = v1.cur_pos - v0.cur_pos;

  *nor = p10.crossProduct(p21);
  *nor /= sqrt(nor->getSqrNorm());

  float face_area = TriangleArea(
    v0.cur_pos,
    v1.cur_pos,
    v2.cur_pos);

  *nor *=face_area;
}

void TriangleMesh::ComputeOriVertexNormals() {
  for (unsigned v_id = 0; v_id < vertices_.size(); ++v_id) {
    Vector3f	&n = vertices_[v_id].ori_nor;
    n[0] = n[1] = n[2] = 0.f;
  }
  Vector3f face_nor;
  for (unsigned f_id = 0; f_id < faces_.size(); ++f_id) {
    const Face &f = faces_[f_id];
    OriginalFaceNormal(f_id, &face_nor);
    vertices_[f.vertex_indices[0]].ori_nor += face_nor;
    vertices_[f.vertex_indices[1]].ori_nor += face_nor;
    vertices_[f.vertex_indices[2]].ori_nor += face_nor;
  }
  for (unsigned v_id = 0; v_id < vertices_.size(); ++v_id) {
    Vector3f	&n = vertices_[v_id].ori_nor;
    n /= sqrt(n.getSqrNorm());
    vertices_[v_id].cur_nor = vertices_[v_id].ori_nor;
  }
}

void TriangleMesh::ComputeCurVertexNormals() {
  for (unsigned v_id = 0; v_id < vertices_.size(); ++v_id) {
    Vector3f	&n = vertices_[v_id].cur_nor;
    n[0] = n[1] = n[2] = 0.f;
  }
  for (unsigned f_id = 0; f_id < faces_.size(); ++f_id) {
    Face &f = faces_[f_id];
    OriginalFaceNormal(f_id, &f.cur_nor);
    vertices_[f.vertex_indices[0]].cur_nor += f.cur_nor;
    vertices_[f.vertex_indices[1]].cur_nor += f.cur_nor;
    vertices_[f.vertex_indices[2]].cur_nor += f.cur_nor;
    f.cur_nor /= sqrt(f.cur_nor.getSqrNorm());
  }
  for (unsigned v_id = 0; v_id < vertices_.size(); ++v_id) {
    Vector3f	&n = vertices_[v_id].cur_nor;
    n /= sqrt(n.getSqrNorm());
  }
}

void TriangleMesh::ComputeFaceNormals() {
  for (unsigned f_id = 0; f_id < faces_.size(); ++f_id) {
    Face &f = faces_[f_id];
    OriginalFaceNormal(f_id, &f.cur_nor);
    f.cur_nor /= sqrt(f.cur_nor.getSqrNorm());
  }
}

void TriangleMesh::ReadVertexTopology(
  vector<int> *sIds, vector<int> *endIds) const {
  int num_vertices_ = static_cast<int> (vertices_.size());
  vector<set<int>> buf_top;
  buf_top.resize(num_vertices_);
  for (unsigned face_id = 0; face_id < faces_.size(); ++face_id) {
    const Face &face = faces_[face_id];
    buf_top[face.vertex_indices[0]].insert(face.vertex_indices[1]);
    buf_top[face.vertex_indices[0]].insert(face.vertex_indices[2]);
    buf_top[face.vertex_indices[1]].insert(face.vertex_indices[0]);
    buf_top[face.vertex_indices[1]].insert(face.vertex_indices[2]);
    buf_top[face.vertex_indices[2]].insert(face.vertex_indices[0]);
    buf_top[face.vertex_indices[2]].insert(face.vertex_indices[1]);
  }

  sIds->resize(num_vertices_+1);
  int iter = 0;
  for (int v_id = 0; v_id < num_vertices_; ++v_id) {
    (*sIds)[v_id] = iter;
    iter += static_cast<int> (buf_top[v_id].size());
  }
  (*sIds)[num_vertices_] = iter;
  endIds->resize(iter);
  iter = 0;
  for (int v_id = 0; v_id < num_vertices_; ++v_id) {
    (*sIds)[v_id] = iter;
    for (set<int>::iterator iter_int = buf_top[v_id].begin();
      iter_int != buf_top[v_id].end();
      ++iter_int) {
      (*endIds)[iter] = *iter_int;
      iter++;
    }
  }
}

void TriangleMesh::OrientNormals() {
  
  // Find disconnected components
  vector<int> s_ids, end_ids;
  ReadVertexTopology(&s_ids, &end_ids);

  vector<bool> point_flags;
  point_flags.resize(vertices_.size());
  for (unsigned pt_id = 0; pt_id < point_flags.size(); ++pt_id)
    point_flags[pt_id] = true;

  vector<vector<int>> clusters;
  unsigned head_id = 0;
  while (true) {
    while (head_id < point_flags.size()) {
      if (point_flags[head_id])
        break;
      head_id++;
    }
    if (head_id == point_flags.size())
      break;

    vector<int> new_cluster;
    new_cluster.push_back(head_id);
    point_flags[head_id] = false;
    int fringe_start = 0, fringe_end = 1;
    while (fringe_start < fringe_end) {
      for (int i = fringe_start; i < fringe_end; ++i) {
        int pt_id = new_cluster[i];
        for (int j = s_ids[pt_id]; j < s_ids[pt_id+1]; ++j) {
          int n_id = end_ids[j];
          if (point_flags[n_id]) {
            point_flags[n_id] = false;
            new_cluster.push_back(n_id);
          }
        }
      }
      fringe_start = fringe_end;
      fringe_end = static_cast<int> (new_cluster.size());
    }
    clusters.push_back(new_cluster);
  }

  // Cluster normals
  vector<Vector3f> cluster_normals;
  cluster_normals.resize(clusters.size());
  int seed_cluster_id = 0;
  int max_cluster_size = 0;
  for (unsigned cluster_id = 0; cluster_id < cluster_normals.size(); ++cluster_id) {
    const vector<int> &cluster = clusters[cluster_id];
    cluster_normals[cluster_id] = vertices_[cluster[0]].ori_nor;
    for (unsigned i = 1; i < cluster.size(); ++i) {
      cluster_normals[cluster_id] += vertices_[cluster[i]].ori_nor;
    }
    cluster_normals[cluster_id].normalize();
    if (static_cast<int> (cluster.size()) > max_cluster_size) {
      max_cluster_size = static_cast<int> (cluster.size());
      seed_cluster_id = cluster_id;
    }
  }

  for (unsigned i = 0; i < point_flags.size(); ++i)
    point_flags[i] = true;

  for (unsigned cluster_id = 0; cluster_id < clusters.size(); ++cluster_id) {
    if (cluster_normals[cluster_id]*cluster_normals[seed_cluster_id] < 0.f) {
      const vector<int> &cluster = clusters[cluster_id];
      for (unsigned i = 1; i < cluster.size(); ++i) {
        vertices_[cluster[i]].ori_nor = -vertices_[cluster[i]].ori_nor;
        vertices_[cluster[i]].cur_nor = -vertices_[cluster[i]].cur_nor;
        point_flags[cluster[i]] = false;
      }
    }
  }
  for (unsigned face_id = 0; face_id < faces_.size(); ++face_id) {
    Face &face = faces_[face_id];
    if (!point_flags[face.vertex_indices[0]]
      ||!point_flags[face.vertex_indices[1]]
      ||!point_flags[face.vertex_indices[2]]) {
      face.cur_nor = -face.cur_nor;
      int tp = face.vertex_indices[1];
      face.vertex_indices[1] = face.vertex_indices[2];
      face.vertex_indices[2] = tp;
    }
  }
}

void TriangleMesh::Read(FILE *file_ptr) {
  ReadVertices(file_ptr);
  ReadFaces(file_ptr);
  bounding_box_.Read(file_ptr);
  LoadString(file_ptr, filename);
}

void TriangleMesh::Write(FILE *file_ptr) {
  WriteVertices(file_ptr);
  WriteFaces(file_ptr);
  bounding_box_.Write(file_ptr);
  SaveString(filename, file_ptr);
}

void TriangleMesh::ReadFaces(FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  faces_.resize(length);
  if (length > 0)
    fread(&faces_[0], sizeof(Face)*length, 1, file_ptr);
}

void TriangleMesh::WriteFaces(FILE *file_ptr) {
  int	length = static_cast<int> (faces_.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&faces_[0], sizeof(Face)*length, 1, file_ptr);
}

void TriangleMesh::ReadQuad(FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  texture_coords_.resize(length);
  if (length > 0)
    fread(&texture_coords_[0], sizeof(Vector3f)*length, 1, file_ptr);
  fread(&length, sizeof(int), 1, file_ptr);
  quad_faces_.resize(length);
  if (length > 0)
    fread(&quad_faces_[0], sizeof(QuadFace)*length, 1, file_ptr);
}

void TriangleMesh::WriteQuad(FILE *file_ptr) {
  int	length = static_cast<int> (texture_coords_.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&texture_coords_[0], sizeof(Vector3f)*length, 1, file_ptr);
  length = static_cast<int> (quad_faces_.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&quad_faces_[0], sizeof(QuadFace)*length, 1, file_ptr);
}