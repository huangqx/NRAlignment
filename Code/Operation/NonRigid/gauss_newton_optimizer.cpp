#include "gauss_newton_optimizer.h"
#include <algorithm>
#include <set>
using namespace std;

void GaussNewtonOptimizer::Compute(
  const OptStructContainer &opt_struct,
  vector<Affine3d> *poses) {
  for (int iteration = 0;
    iteration < max_num_of_gn_iterations_;
    ++iteration) {
    QuadraticForm quad_form;
    GenerateQuadraticForm(opt_struct, *poses, &quad_form);
    vector<Vector6d> velocities;
    ConjugateGradientDescent(quad_form, &velocities);
    for (unsigned pose_id = 0; pose_id < poses->size(); ++pose_id) {
      Affine3d motion(&velocities[pose_id][0]);
      (*poses)[pose_id] = motion*(*poses)[pose_id];
    }
  }
}

void GaussNewtonOptimizer::GenerateQuadraticForm(
  const OptStructContainer &opt_struct,
  const vector<Affine3d> &poses,
  QuadraticForm *quad_form) {
  int num_poses = static_cast<int> (poses.size());
  // Compute jacobi matrices
  vector<Matrix12x6d> jacobis;
  jacobis.resize(num_poses);
  vector<Vector12d> x0;
  x0.resize(poses.size());

  for (int pose_id = 0; pose_id < num_poses; ++pose_id) {
    const Affine3d &pose = poses[pose_id];
    Matrix12x6d &jacobi = jacobis[pose_id];
    for (int i = 0; i < 6; ++i)
      for (int j = 0; j < 12; ++j)
        jacobi[i][j] = 0.0;

    for (int i = 0; i < 3; ++i)
      jacobi[i][i] = 1.0;
      
    const Vector3d &b = pose[0];
    jacobi[4][0] = b[2]; jacobi[5][0] = -b[1];
    jacobi[3][1] = -b[2]; jacobi[5][1] = b[0];
    jacobi[3][2] = b[1]; jacobi[4][2] = -b[0]; 
    const Vector3d &a1 = pose[1];
    jacobi[4][3] = a1[2]; jacobi[5][3] = -a1[1];
    jacobi[3][4] = -a1[2]; jacobi[5][4] = a1[0];
    jacobi[3][5] = a1[1]; jacobi[4][5] = -a1[0];
    const Vector3d &a2 = pose[2];
    jacobi[4][6] = a2[2]; jacobi[5][6] = -a2[1];
    jacobi[3][7] = -a2[2]; jacobi[5][7] = a2[0];
    jacobi[3][8] = a2[1]; jacobi[4][8] = -a2[0];
    const Vector3d &a3 = pose[3];
    jacobi[4][9] = a3[2]; jacobi[5][9] = -a3[1];
    jacobi[3][10] = -a3[2]; jacobi[5][10] = a3[0];
    jacobi[3][11] = a3[1]; jacobi[4][11] = -a3[0];

    for (int k = 0; k < 3; ++k) {
      x0[pose_id][k] = pose[0][k];
      x0[pose_id][k+3] = pose[1][k];
      x0[pose_id][k+6] = pose[2][k];
      x0[pose_id][k+9] = pose[3][k];
    }
  }

  quad_form->c = 0.0;
  quad_form->vecb.resize(num_poses);
  quad_form->matA.resize(num_poses);
  for (int i = 0; i < num_poses; ++i)
    for (int k = 0; k < 6; ++k)
      quad_form->vecb[i][k] = 0.0;

  // Non diagonal parts
  const vector<NonDiagonalTerm> *non_diag_terms =
    opt_struct.GetReguNonDiagonalTerms();

  Matrix12d A_12;
  for (unsigned ele_id = 0; ele_id < non_diag_terms->size(); ++ele_id) {
    const NonDiagonalTerm &ndterm = (*non_diag_terms)[ele_id];
    for (int i = 0; i < 12; ++i)
      for (int j = 0; j < 12; ++j)
        A_12[i][j] = 0.0;

    for (int i = 0; i < 4; ++i)
      for (int j = 0; j < 4; ++j)
        for (int k = 0; k < 3; ++k)
          A_12[3*i+k][3*j+k] = ndterm.matA[i][j];
        
    const Vector12d &x_i = x0[ndterm.row_id];
    const Vector12d &x_j = x0[ndterm.col_id];

    const Matrix12x6d &J_i = jacobis[ndterm.row_id];
    const Matrix12x6d &J_j = jacobis[ndterm.col_id];

    Matrix6d m_ij = ((J_i.transpose())*A_12*J_j)*smoothness_weight_;
    quad_form->matA[ndterm.row_id].InsertABlock(ndterm.col_id, m_ij);
    quad_form->vecb[ndterm.row_id] -= (J_i.transpose())*(A_12*x_j)*smoothness_weight_/2;
    quad_form->vecb[ndterm.col_id] -= (J_j.transpose())*(A_12*x_i)*smoothness_weight_/2;
    quad_form->c += x_i*(A_12*x_j)*smoothness_weight_;
  }

  const vector<Matrix4d> *diag_terms = opt_struct.GetReguDiagonalTerms();
  const vector<PatchQuadraticTerm> *data_terms =
    opt_struct.GetPatchQuadraticTerms();

  for (int i = 0; i < num_poses; ++i) {
    const PatchQuadraticTerm &pqt = (*data_terms)[i];
    const Matrix4d &A_4 = (*diag_terms)[i];
    for (int j = 0; j < 12; ++j) {
      for (int k = 0; k < 12; ++k)
        A_12[j][k] = pqt.matA[j][k];
      A_12[j][j] = pqt.matA[j][j] + 1e-4;
    }
    for (int j = 0; j < 4; ++j)
      for (int k = 0; k < 4; ++k)
        for (int l = 0; l < 3; ++l)
          A_12[3*j+l][3*k+l] += A_4[j][k]*smoothness_weight_;

    const Vector12d &x_i = x0[i];
    const Matrix12x6d &J_i = jacobis[i];

    Matrix6d m_ii = (J_i.transpose())*A_12*J_i;
    Vector6d b_i = (J_i.transpose())*(pqt.vecb - A_12*x_i);
    double c = pqt.c + x_i*(A_12*x_i) -2*(pqt.vecb*x_i);

    quad_form->matA[i].InsertABlock(i, m_ii);
    quad_form->vecb[i] += b_i;
    quad_form->c += c;
  }

  quad_form->diagonal_entries.resize(num_poses);
  for (unsigned row_id = 0; row_id < quad_form->matA.size(); ++row_id) {
    const RowBlock &row_block = quad_form->matA[row_id];  
    for (unsigned i = 0; i < row_block.ele_col_ids.size(); ++i) {
      int col_id = row_block.ele_col_ids[i];
      if (col_id == row_id) {
        for (int j = 0; j < 6; ++j) {
          quad_form->diagonal_entries[row_id][j] = row_block.eles[i][j][j];
        }
      }
    }
  }
}

void GaussNewtonOptimizer::ConjugateGradientDescent(
  const QuadraticForm &quad_form,
  vector<Vector6d> *x) {
  int iteration = 0;
  int block_dim = static_cast<int> (quad_form.vecb.size());
  // x
  x->resize(block_dim);
  for (int i = 0; i < block_dim; ++i) {
    for (int j = 0; j < 6; ++j) {
      (*x)[i][j] = 0.0;
    }
  }
  // r
  vector<Vector6d> vec_r = quad_form.vecb;

  // d
  vector<Vector6d> vec_d = vec_r;
  Normalize(quad_form.diagonal_entries, &vec_d);

  double delta_new = InnerProduct(vec_r, vec_d);
  double delta_0 = delta_new;

  // q
  vector<Vector6d> vec_q, vec_s;
  vec_q.resize(block_dim);
  vec_s.resize(block_dim);

  while (iteration < max_num_of_cg_iterations_
    && delta_new > epsilon_max_*epsilon_max_*delta_0) {
    MatrixMultiplication(quad_form, vec_d, &vec_q);

    double alpha = delta_new/InnerProduct(vec_d, vec_q);
    for (int i = 0; i < block_dim; ++i)
      (*x)[i] += vec_d[i]*alpha;

    if (iteration % 50 == 0) {
      MatrixMultiplication(quad_form, *x, &vec_r);
      for (int i = 0; i < block_dim; ++i)
        vec_r[i] = quad_form.vecb[i] - vec_r[i];
    } else {
      for (int i = 0; i < block_dim; ++i)
        vec_r[i] -= vec_q[i]*alpha;
    }
    vec_s = vec_r;
    Normalize(quad_form.diagonal_entries, &vec_s);
    double delta_old = delta_new;
    delta_new = InnerProduct(vec_r, vec_s);

    double beta = delta_new/delta_old;
    for (int i = 0; i < block_dim; ++i)
      vec_d[i] = vec_s[i] + vec_d[i]*beta;

    iteration = iteration + 1;
  }
}

void GaussNewtonOptimizer::MatrixMultiplication(
  const QuadraticForm &quad_form,
  const vector<Vector6d> &x,
  vector<Vector6d> *Ax) {
  Ax->resize(x.size());
  int block_dim = static_cast<int> (x.size());
  
  for (int row_id = 0; row_id < block_dim; ++row_id) {
    Vector6d &result = (*Ax)[row_id];
    for (int i = 0; i < 6; ++i)
      result[i] = 0.0;
    const RowBlock &row_struct = quad_form.matA[row_id];
   
    for (unsigned i = 0; i < row_struct.eles.size(); ++i)
      result += row_struct.eles[i]*x[row_struct.ele_col_ids[i]];
  }
}

void GaussNewtonOptimizer::Normalize(
  const vector<Vector6d> &diag_entries,
  vector<Vector6d> *x) {
  if (x->size() == diag_entries.size()) {
    for (unsigned i = 0; i < diag_entries.size(); ++i) {
      for (int k = 0; k < 6; ++k) {
        (*x)[i][k] /= diag_entries[i][k];
      }
    }
  }
}

double GaussNewtonOptimizer::InnerProduct(
  const vector<Vector6d> &vec1,
  const vector<Vector6d> &vec2) {
  double sum = 0.0;
  if (vec1.size() != vec2.size())
    return 0.0;

  for (unsigned i = 0; i < vec1.size(); ++i)
    sum += vec1[i]*vec2[i];
  return sum;
}