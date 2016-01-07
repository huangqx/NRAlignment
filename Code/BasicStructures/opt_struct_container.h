#ifndef opt_struct_container_h_
#define opt_struct_container_h_

#include "linear_algebra_templcode.h"
#include "linear_algebra.h"
#include <vector>
using namespace std;

struct PatchQuadraticTerm {
 public:
  PatchQuadraticTerm() {
  }
  ~PatchQuadraticTerm() {
  }
  void SetZero() {
    for (int i = 0; i < 12; ++i) {
      for (int j = 0; j < 12; ++j) {
        matA[i][j] = 0.0;
      }
      vecb[i] = 0.0;
    }
    c = 0.0;
  }
  Matrix12d matA;
  Vector12d vecb;
  double c;
};

struct NonDiagonalTerm {
 public:
  NonDiagonalTerm() {
  }
  ~NonDiagonalTerm() {
  }
  int row_id;
  int col_id;
  Matrix4d matA;
};

class OptStructContainer {
 public:
  OptStructContainer() {
  }
  ~OptStructContainer() {
  }
  void Read(FILE *file_ptr);
  void Write(FILE *file_ptr);
  void Save();
  vector<PatchQuadraticTerm>* GetPatchQuadraticTerms() {
    return &data_terms_;
  }
  const vector<PatchQuadraticTerm>* GetPatchQuadraticTerms() const {
    return &data_terms_;
  }
  vector<Matrix4d>* GetReguDiagonalTerms() {
    return &regu_diag_terms_;
  }
  const vector<Matrix4d>* GetReguDiagonalTerms() const {
    return &regu_diag_terms_;
  }
  vector<NonDiagonalTerm>* GetReguNonDiagonalTerms() {
    return &regu_nondiag_terms_;
  }
  const vector<NonDiagonalTerm>* GetReguNonDiagonalTerms() const {
    return &regu_nondiag_terms_;
  }
 private:
  vector<PatchQuadraticTerm> data_terms_;
  vector<Matrix4d> regu_diag_terms_;
  vector<NonDiagonalTerm> regu_nondiag_terms_;
};

#endif