#include "multile_rigid_alignment.h"
#include <algorithm>

void MultipleRigidAlign::LoopClosure(vector<Affine3d> *adjRigidTranforms) {
  int numScans = static_cast<int> (adjRigidTranforms->size());
  // Compute optimized relative motions
  vector<Matrix3d> rotations;
  rotations.resize(numScans);
  for (int scan_id = 0; scan_id < numScans; ++scan_id) {
    rotations[scan_id][0] = (*adjRigidTranforms)[scan_id][1];
    rotations[scan_id][1] = (*adjRigidTranforms)[scan_id][2];
    rotations[scan_id][2] = (*adjRigidTranforms)[scan_id][3];
    rotations[scan_id] = rotations[scan_id].transpose();
  }

  for (int scan_id = 0; scan_id < numScans; ++scan_id) {
    Matrix3d rotError;
    // Initialize as the identity matrix
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j)
        rotError[i][j] = 0.0;
      rotError[i][i] = 1.0;
    }

    for (int i = scan_id; i < numScans; ++i) {
      rotError = rotations[i]*rotError;
      RectifyRotationMatrix(&rotError);
    }
   
    for (int i = 0; i < scan_id; ++i) {
      rotError = rotations[i]*rotError;
      RectifyRotationMatrix(&rotError);
    }
    
    RotationMatrixPower(1.0/numScans, &rotError);

    // Correct the rotational error
    Matrix3d newRotation = rotError*rotations[scan_id].transpose();

    (*adjRigidTranforms)[scan_id][1] = newRotation[0];
    (*adjRigidTranforms)[scan_id][2] = newRotation[1];
    (*adjRigidTranforms)[scan_id][3] = newRotation[2];
  }

  // Correct translational error
  Vector3d accumTransError;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j)
      rotations[0][i][j] = 0.0;
    rotations[0][i][i] = 1.0;
    accumTransError[i] = 0.0;
  }

  for (int scan_id = 0; scan_id < numScans; ++scan_id) {
    accumTransError += rotations[scan_id]*(*adjRigidTranforms)[scan_id][0];
    if (scan_id < numScans-1) {
      rotations[scan_id+1][0] = (*adjRigidTranforms)[scan_id][1];
      rotations[scan_id+1][1] = (*adjRigidTranforms)[scan_id][2];
      rotations[scan_id+1][2] = (*adjRigidTranforms)[scan_id][3];
      rotations[scan_id+1] = rotations[scan_id]*rotations[scan_id+1];
      RectifyRotationMatrix(&rotations[scan_id+1]);
    }
  }
  accumTransError /= numScans;

  for (int scan_id = 0; scan_id < numScans; ++scan_id) {
    (*adjRigidTranforms)[scan_id][0] -=
      rotations[scan_id].transpose()*accumTransError;
  }
}

void MultipleRigidAlign::RotationMatrixPower(
  const double &alpha, Matrix3d *rot) {
  Vector3d axis;
  axis[0] = (*rot)[2][1] - (*rot)[1][2];
  axis[1] = (*rot)[0][2] - (*rot)[2][0];
  axis[2] = (*rot)[1][0] - (*rot)[0][1];
  axis /= sqrt(axis*axis);

  double t = (3-(*rot)[0][0]-(*rot)[1][1]-(*rot)[2][2])/4;
  double costheta = sqrt(1-t), sintheta = sqrt(t);

  double theta = acos(costheta);
  if (sintheta < 0)
    theta = -theta;

  theta = theta*alpha;
  costheta = cos(theta);
  sintheta = sin(theta);

  Vector4d quat;
  quat[0] = costheta;
  quat[1] = sintheta*axis[0];
  quat[2] = sintheta*axis[1];
  quat[3] = sintheta*axis[2];

  (*rot)[0][0] = 1 - 2*(quat[2]*quat[2]+quat[3]*quat[3]);
  (*rot)[1][1] = 1 - 2*(quat[1]*quat[1]+quat[3]*quat[3]);
  (*rot)[2][2] = 1 - 2*(quat[1]*quat[1]+quat[2]*quat[2]);

  (*rot)[1][0] = 2*(quat[1]*quat[2] + quat[0]*quat[3]);
  (*rot)[0][1] = 2*(quat[1]*quat[2] - quat[0]*quat[3]);

  (*rot)[2][0] = 2*(quat[1]*quat[3] - quat[0]*quat[2]);
  (*rot)[0][2] = 2*(quat[1]*quat[3] + quat[0]*quat[2]);

  (*rot)[2][1] = 2*(quat[2]*quat[3] + quat[0]*quat[1]);
  (*rot)[1][2] = 2*(quat[2]*quat[3] - quat[0]*quat[1]);
}

void MultipleRigidAlign::ApplyOptimizedRigidPoses(const vector<Affine3d> &adjRigidTranforms,
  vector<ShapeContainer> *inputShapes) {
  Affine3d curTransform = adjRigidTranforms[0];
  SetCurrentPose(curTransform, &(*inputShapes)[1].surface);
  for (unsigned i = 1; i < inputShapes->size()-1; ++i) {
    curTransform = curTransform*adjRigidTranforms[i];
    RectifyRotationalPart(&curTransform);
    SetCurrentPose(curTransform, &(*inputShapes)[i+1].surface);
  }
}

void MultipleRigidAlign::SetCurrentPose(
  const Affine3d &rigidTransform, TriangleMesh *mesh) {
  for (unsigned v_id = 0; v_id < mesh->GetVertexArray()->size(); ++v_id) {
    Vertex &v = (*mesh->GetVertexArray())[v_id];
    Vector3d cur_pos = rigidTransform[0]
    + rigidTransform[1]*v.ori_pos[0]
    + rigidTransform[2]*v.ori_pos[1]
    + rigidTransform[3]*v.ori_pos[2];
    Vector3d cur_nor = rigidTransform[1]*v.ori_nor[0]
    + rigidTransform[2]*v.ori_nor[1]
    + rigidTransform[3]*v.ori_nor[2];
    for (int i = 0; i < 3; ++i) {
      v.cur_pos[i] = static_cast<float> (cur_pos[i]);
      v.cur_nor[i] = static_cast<float> (cur_nor[i]);
    }
  }
  mesh->ComputeFaceNormals();
}

void MultipleRigidAlign::RectifyRotationMatrix(Matrix3d *rot) {
  (*rot)[0] /= sqrt((*rot)[0].getSqrNorm());
  (*rot)[1] -= (*rot)[0]*((*rot)[0]*(*rot)[1]);
  (*rot)[1] /= sqrt((*rot)[1].getSqrNorm());
  (*rot)[2] -= (*rot)[0]*((*rot)[0]*(*rot)[2])
    + (*rot)[1]*((*rot)[1]*(*rot)[2]);
  (*rot)[2] /= sqrt((*rot)[2].getSqrNorm());
}

void MultipleRigidAlign::RectifyRotationalPart(Affine3d *rigid) {
  (*rigid)[1] /= sqrt((*rigid)[1].getSqrNorm());
  (*rigid)[2] -= (*rigid)[1]*((*rigid)[1]*(*rigid)[2]);
  (*rigid)[2] /= sqrt((*rigid)[2].getSqrNorm());
  (*rigid)[3] -= (*rigid)[1]*((*rigid)[1]*(*rigid)[3])
    + (*rigid)[2]*((*rigid)[2]*(*rigid)[3]);
  (*rigid)[3] /= sqrt((*rigid)[3].getSqrNorm());
}