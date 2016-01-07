#include "point_cloud.h"

Vertex::Vertex() {
  flags = 0;
  density = 0.f;
  iso_value = 0.f;
  ori_pos[0] = ori_pos[1] = ori_pos[2] = 0.f;
  ori_nor[0] = ori_nor[1] = ori_nor[2] = 0.f;
  cur_pos = ori_pos;
  cur_nor = ori_nor;
  color[0] = color[1] = color[2] = 0.f;
}

Vertex::~Vertex() {
}

int Vertex::GetMaterial() const{
  return flags & 255;
}

void Vertex::SetMaterial(int material) {
  material = material & 255;
  flags = flags | 255;
  flags -= 255 - material;
}

bool Vertex::GetFlag() const{
  return (flags & VERTEX_FLAG_BIT) == VERTEX_FLAG_BIT;
}

void Vertex::SetFlag(bool f) {
  if(f) {
    flags = flags | VERTEX_FLAG_BIT;
  } else {
    flags = flags & (~VERTEX_FLAG_BIT);
  }
}

bool Vertex::GetIsBoundaryFlag() const{
  return (flags & VERTEX_ISBOUNDARY_BIT) == VERTEX_ISBOUNDARY_BIT;
}

void Vertex::SetIsBoundaryFlag(bool f) {
  if(f) {
    flags = flags | VERTEX_ISBOUNDARY_BIT;
  } else {
    flags = flags & (~VERTEX_ISBOUNDARY_BIT);
  }
} 

bool Vertex::GetIsFeatureFlag() const {
  return (flags & VERTEX_ISFEATURE_BIT) == VERTEX_ISFEATURE_BIT;
}

void Vertex::SetIsFeatureFlag(bool f) {
  if (f) {
    flags = flags | VERTEX_ISFEATURE_BIT;
  } else {
    flags = flags & (~VERTEX_ISFEATURE_BIT);
  }
}

bool Vertex::GetIsActiveFlag() const {
  return (flags & VERTEX_ISACTIVE_BIT) == VERTEX_ISACTIVE_BIT;
}

void Vertex::SetIsActiveFlag(bool f) {
  if (f) {
    flags = flags | VERTEX_ISACTIVE_BIT;
  } else {
    flags = flags & (~VERTEX_ISACTIVE_BIT);
  }
}

void PointCloud::ComputeBoundingBox() {
  bounding_box_.Initialize();
  for (unsigned v_id = 0; v_id < vertices_.size(); ++v_id) {
    Vertex &vertex = vertices_[v_id];
    bounding_box_.Insert_A_Point(vertex.ori_pos);
  }
}

void PointCloud::Read(FILE *file_ptr) {
  ReadVertices(file_ptr);
  bounding_box_.Read(file_ptr);
}

void PointCloud::Write(FILE *file_ptr) {
  WriteVertices(file_ptr);
  bounding_box_.Write(file_ptr);
}

void PointCloud::ReadVertices(FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vertices_.resize(length);
  if (length > 0)
    fread(&vertices_[0], sizeof(Vertex)*length, 1, file_ptr);
}

void	PointCloud::WriteVertices(FILE *file_ptr) {
  int	length = static_cast<int> (vertices_.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vertices_[0], sizeof(Vertex)*length, 1, file_ptr);
}

