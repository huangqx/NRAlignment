#ifndef gauss_newton_optimizer_h_
#define gauss_newton_optimizer_h_

#include "affine_transformation_templcode.h"
#include "affine_transformation.h"
#include "dynamic_linear_algebra_templcode.h"
#include "dynamic_linear_algebra.h"
#include "opt_struct_container.h"
#include <vector>
using namespace std;

struct RowBlock {
 public:
  inline RowBlock() {
  }
  ~RowBlock() {
  }
  void InsertABlock(int col_id, const Matrix6d &mat) {
    eles.push_back(mat);
    ele_col_ids.push_back(col_id);
  }
  vector<Matrix6d> eles;
  vector<int> ele_col_ids;
};

struct QuadraticForm {
  QuadraticForm() {
  }
  ~QuadraticForm() {
  }
  double c;
  vector<Vector6d> vecb;
  vector<RowBlock> matA;

  vector<Vector6d> diagonal_entries;
};

class GaussNewtonOptimizer {
 public:
  GaussNewtonOptimizer() {
    // You don't want to change these parameters
    max_num_of_gn_iterations_ = 4;
    max_num_of_cg_iterations_ = 200;
    epsilon_max_ = 1e-4;
    smoothness_weight_ = 0.1;
  }
  ~GaussNewtonOptimizer() {
  }
  void Compute(const OptStructContainer &opt_struct,
    vector<Affine3d> *poses);
  void SetSmoothnessWeight(double weight) {
    smoothness_weight_ = weight;
  }
 private:
  void GenerateQuadraticForm(const OptStructContainer &opt_struct,
    const vector<Affine3d> &poses,
    QuadraticForm *quad_form);
  void ConjugateGradientDescent(const QuadraticForm &quad_form,
    vector<Vector6d> *x);
  void MatrixMultiplication(const QuadraticForm &quad_form,
    const vector<Vector6d> &x,
    vector<Vector6d> *Ax);
  void Normalize(const vector<Vector6d> &diag_entries,
    vector<Vector6d> *x);
  double InnerProduct(const vector<Vector6d> &vec1,
    const vector<Vector6d> &vec2);
  int max_num_of_gn_iterations_;
  int max_num_of_cg_iterations_;
  double epsilon_max_;
  double smoothness_weight_;
};

#endif