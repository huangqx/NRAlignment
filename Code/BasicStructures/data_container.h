#ifndef data_container_h_
#define data_container_h_

#include "stdafx.h"
#include "opt_struct_container.h"
#include "patch_container.h"
#include <string>
using namespace std;

#include "triangle_mesh.h"

struct ShapeContainer {
 public:
  ShapeContainer() {
  }
  ~ShapeContainer() {
  }
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);
  void GenerateCurrentSurface(const PatchContainer &pc,
    const bool &blend);
  void ApplyGlobalTransform(const Vector3f &global_translation,
    const float &global_scaling);
  TriangleMesh surface;
};

class COptionView;
class DataContainer {
 public:
  DataContainer() {
	}
	~DataContainer() {
	}

	/************************************************************************/
	/* IO Operations
	*/
	/************************************************************************/
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);

  void ComputeBoundingBox();
  void Normalize();
  Vector3f GetGlobalTranslation() {
    return global_translation_;
  }
  float GetGlobalScaling() {
    return global_scaling_;
  }

  void ClearRenderingFlags();
  bool SetRenderingFlags(const CString &itemName);
  void SetNames(COptionView *view);
  void SetShapeNames(COptionView *view);

  /************************************************************************/
  /* Access functions
  */
  /************************************************************************/
  vector<ShapeContainer> *GetInputShapes() {
    return &input_shapes_;
  }
  PointCloud *GetReferenceModel() {
    return &reference_model_;
  }

  BoundingBox* GetBoundingBox() {
    return &bounding_box_;
  }

  /************************************************************************/
  /* Update view point
  */
  /************************************************************************/
  bool first_view_;

  /************************************************************************/
  /* Access functions
  /************************************************************************/
  bool CanLockScan(CString itemName);
  bool CanUnLockScan(CString itemName);

  bool LockScan(CString itemName);
  bool UnLockScan(CString itemName);
  void LockSelected();
  void UnLockSelected();

  // Remove scans
  bool DeleteScan(CString itemName);
  void DeleteSelected();
 private:
  // shapes to be matched
  vector<ShapeContainer> input_shapes_;

  // The reference model
  PointCloud reference_model_;

  // bounding box of the whole thing
  BoundingBox bounding_box_;

  // Transformation applied during normalization
  Vector3f global_translation_;
  float global_scaling_;
};


#endif