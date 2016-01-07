#ifndef triangle_mesh_h_
#define triangle_mesh_h_

#include <string>
#include <vector>

#include "point_cloud.h"
using namespace std;
struct Face {
  Face() {
    flags = 12;
  }
  ~Face() {
  }

  int GetMaterial() const {
    return flags & 255;
  }
  void SetMaterial(int material) {
    material = material & 255;
    flags = flags | 255;
    flags -= 255 - material;
  }
  int GetPatchIndex() const {
    return flags>>16;
  }
  void SetPatchIndex(int id) {
    int i1 = id<<16;
    int i2 = flags &((1<<16)-1);
    flags = i1 + i2;
  }
//  Vector3f ori_nor;
  Vector3f cur_nor;
  int vertex_indices[3];
  int flags;
};

// Only used in storage
struct QuadFace {
 public:
  QuadFace() {
    v_ids[0] = v_ids[1] = v_ids[2] = v_ids[3] = -1;
    vt_ids[0] = vt_ids[1] = vt_ids[2] = vt_ids[3] = -1;
  }
  ~QuadFace() {
  }
  int v_ids[4];
  int vt_ids[4];
};


class TriangleMesh:	public PointCloud {
 public:
  TriangleMesh();
  ~TriangleMesh();

  void Clear();
  // Compute normal direction for vertices at original vertex positions
  void ComputeOriVertexNormals();
  // Compute normal direction for vertices at current vertex positions
  void ComputeCurVertexNormals();
  // Compute normal direction for faces at current vertex positions
  void ComputeFaceNormals();
  // Orient normal direction
  void OrientNormals();
  
  /************************************************************************/
  /* Neighbor query
  /************************************************************************/
  void ReadVertexTopology(vector<int> *sIds, vector<int> *endIds) const;
 
	/************************************************************************/
	/* IO to binary files
	/************************************************************************/
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);

  /************************************************************************/
  /* Access functions
  /************************************************************************/
  vector<Face>* GetFaceArray() {
    return &faces_;
  }
  const vector<Face>* GetFaceArray() const {
    return &faces_;
  }
  vector<Vector3f>* GetTextureCoords() {
    return &texture_coords_;
  }
  const vector<Vector3f>* GetTextureCoords() const {
    return &texture_coords_;
  }
  vector<QuadFace>* GetQuadFaces() {
    return &quad_faces_;
  }
  const vector<QuadFace>* GetQuadFaces() const {
    return &quad_faces_;
  }
 private:
  void OriginalFaceNormal(int face_id, Vector3f *nor) const ;
  void CurrentFaceNormal(int face_id, Vector3f *nor) const ;
  void ReadFaces(FILE *file_ptr);
  void WriteFaces(FILE *file_ptr);
  void ReadQuad(FILE *file_ptr);
  void WriteQuad(FILE *file_ptr);
  vector<Face> faces_;

  // Used for quad-meshes
  vector<Vector3f> texture_coords_;
  vector<QuadFace> quad_faces_;
};

#endif