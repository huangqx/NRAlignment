#include "opt_struct_container.h"
#include "io_helper.h"

void OptStructContainer::Read(FILE *file_ptr) {
  int num;
  fread(&num, sizeof(int), 1, file_ptr);
  data_terms_.clear();
  data_terms_.resize(num);
  if (num > 0)
    fread(&data_terms_[0], sizeof(PatchQuadraticTerm)*num, 1, file_ptr);
  fread(&num, sizeof(int), 1, file_ptr);
  regu_diag_terms_.clear();
  regu_diag_terms_.resize(num);
  if (num > 0)
    fread(&regu_diag_terms_[0], sizeof(Matrix4d)*num, 1, file_ptr);
  fread(&num, sizeof(int), 1, file_ptr);
  regu_nondiag_terms_.clear();
  regu_nondiag_terms_.resize(num);
  if (num > 0)
    fread(&regu_nondiag_terms_[0], sizeof(NonDiagonalTerm)*num, 1, file_ptr);
}

void OptStructContainer::Write(FILE *file_ptr) {
  int num = static_cast<int> (data_terms_.size());
  fwrite(&num, sizeof(int), 1, file_ptr);
  if (num > 0)
    fwrite(&data_terms_[0], sizeof(PatchQuadraticTerm)*num, 1, file_ptr);
  num = static_cast<int> (regu_diag_terms_.size());
  fwrite(&num, sizeof(int), 1, file_ptr);
  if (num > 0)
    fwrite(&regu_diag_terms_[0], sizeof(Matrix4d)*num, 1, file_ptr);
  num = static_cast<int> (regu_nondiag_terms_.size());
  fwrite(&num, sizeof(int), 1, file_ptr);
  if (num > 0)
    fwrite(&regu_nondiag_terms_[0], sizeof(NonDiagonalTerm)*num, 1, file_ptr);
}

void OptStructContainer::Save() {
  vector<int> A_rows, A_cols;
  vector<double> A_vals;
  for (unsigned i = 0; i < regu_diag_terms_.size(); ++i) {
    for (int j = 0; j < 4; ++j) {
      for (int k = 0; k < 4; ++k) {
        A_rows.push_back(4*i+j+1);
        A_cols.push_back(4*i+k+1);
        A_vals.push_back(regu_diag_terms_[i][k][j]);
      }
    }
  }
  for (unsigned i = 0; i < regu_nondiag_terms_.size(); ++i) {
    const NonDiagonalTerm &nd_term = regu_nondiag_terms_[i];
    for (int j = 0; j < 4; ++j) {
      for (int k = 0; k < 4; ++k) {
        A_rows.push_back(4*nd_term.row_id+j+1);
        A_cols.push_back(4*nd_term.col_id+k+1);
        A_vals.push_back(nd_term.matA[k][j]);
      }
    }
  }
  vector<double> data;
  for (unsigned i = 0; i < data_terms_.size(); ++i) {
    PatchQuadraticTerm &pq_term = data_terms_[i];
    for (int j = 0; j < 12; ++j) {
      for (int k = 0; k < 12; ++k) {
        data.push_back(pq_term.matA[k][j]);
      }
    }
    for (int j = 0; j < 12; ++j)
      data.push_back(pq_term.vecb[j]);
    data.push_back(pq_term.c);
  }
  FILE *file_ptr;
  fopen_s(&file_ptr, "regu.txt", "w");
  for (unsigned i = 0; i < A_rows.size(); ++i) {
    fprintf_s(file_ptr, "%d %d %.20f\n",
      A_rows[i], A_cols[i], A_vals[i]);
  }
  fclose(file_ptr);

  fopen_s(&file_ptr, "data.txt", "w");
  for (unsigned i = 0; i < data.size(); ++i) {
    fprintf_s(file_ptr, "%.20f\n",
      data[i]);
  }
  fclose(file_ptr);
}