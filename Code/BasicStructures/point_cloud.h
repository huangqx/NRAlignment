#ifndef point_cloud_h_
#define point_cloud_h_

#include "Surface.h"

#include <vector>
using	namespace	std;

const unsigned int VERTEX_FLAG_BIT = 1<<8;
const unsigned int VERTEX_ISBOUNDARY_BIT = 1<<9;
const unsigned int VERTEX_ISSEED_BIT = 1<<10;
const unsigned int VERTEX_ISFEATURE_BIT	= 1<<11;
const unsigned int VERTEX_ISACTIVE_BIT = 1<<12;

struct Vertex {
public:
  Vertex();
  ~Vertex();

  int GetMaterial() const;
  void SetMaterial(int material);
  bool GetFlag() const ;
  void SetFlag(bool f);
  bool GetIsFeatureFlag() const;
  void SetIsFeatureFlag(bool f);
  bool GetIsBoundaryFlag() const ;
  void SetIsBoundaryFlag(bool f);
  bool GetIsActiveFlag() const;
  void SetIsActiveFlag(bool f);

  /************************************************************************/
  /* position
  /************************************************************************/
  Vector3f ori_pos;
  Vector3f cur_pos;


  /************************************************************************/
  /* normal
  /************************************************************************/
  Vector3f ori_nor;
  Vector3f cur_nor;

  /************************************************************************/
  /* color 
  /************************************************************************/
  Vector3f color;

  /************************************************************************/
  /* Sampling density around this point
  /************************************************************************/
  float density;

  /************************************************************************/
  /* iso value
  /************************************************************************/
  float iso_value;

  /************************************************************************/
  /* flags
  /************************************************************************/
  int flags;
};

class PointCloud: public Surface {
 public:
	PointCloud(){
  }
	virtual ~PointCloud(){
  }
  
  /************************************************************************/
	 /* Load and read from binary files
	 /************************************************************************/
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);
  
  // Bounding box
  void ComputeBoundingBox();

  // Access functions
  vector<Vertex>* GetVertexArray() {
    return &vertices_;
  }
  const vector<Vertex>* GetVertexArray() const {
    return &vertices_;
  }
 protected:
  void ReadVertices(FILE *file_ptr);
  void WriteVertices(FILE *file_ptr);
  vector<Vertex> vertices_; //vertex array
};

#endif