#ifndef multiple_rigid_h_
#define multiple_rigid_h_

#include "data_container.h"
#include "affine_transformation_templcode.h"
#include "affine_transformation.h"
#include "ANN.h"

#include <vector>
using namespace std;

class MultipleRigidAlign {
 public:
  MultipleRigidAlign() {
  }
  ~MultipleRigidAlign() {
  }
  void LoopClosure(vector<Affine3d> *adjRigidTranforms);
  void ApplyOptimizedRigidPoses(const vector<Affine3d> &adjRigidTranforms,
    vector<ShapeContainer> *inputShapes);
 private:
  void SetCurrentPose(const Affine3d &rigidTransform,
    TriangleMesh *mesh);
  void RotationMatrixPower(const double &alpha,
    Matrix3d *rot);
  void RectifyRotationMatrix(Matrix3d *rot);
  void RectifyRotationalPart(Affine3d *rigid);
};

#endif