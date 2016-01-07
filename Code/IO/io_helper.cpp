#include "io_helper.h"

void LoadIntegerVector(FILE *file_ptr, vector<int> *vec) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vec->resize(length);
  if (length > 0)
    fread(&(*vec)[0], sizeof(int)*length, 1, file_ptr);
}

void SaveIntegerVector(const vector<int> &vec, FILE *file_ptr) {
  int	length = static_cast<int> (vec.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vec[0], sizeof(int)*length, 1, file_ptr);
}
/*
void LoadFloatVector(vector<float> &vec, FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vec.resize(length);
  if (length > 0)
    fread(&vec[0], sizeof(float)*length, 1, file_ptr);
}

void SaveFloatVector(vector<float> &vec, FILE *file_ptr) {
  int		length = static_cast<int> (vec.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vec[0], sizeof(float)*length, 1, file_ptr);
}

void LoadIDoubleVector(vector<double> &vec, FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vec.resize(length);
  if (length > 0)
    fread(&vec[0], sizeof(double)*length, 1, file_ptr);
}

void SaveDoubleVector(vector<double> &vec, FILE *file_ptr) {
  int		length = static_cast<int> (vec.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vec[0], sizeof(double)*length, 1, file_ptr);
}
void LoadFloat3Vector(vector<Vector3f> &vec, FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vec.resize(length);
  if (length > 0)
    fread(&vec[0], sizeof(Vector3f)*length, 1, file_ptr);
}

void SaveFloat3Vector(vector<Vector3f> &vec, FILE *file_ptr) {
  int		length = static_cast<int> (vec.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vec[0], sizeof(Vector3f)*length, 1, file_ptr);
}

void LoadIDouble3Vector(vector<Vector3d> &vec, FILE *file_ptr) {
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  vec.resize(length);
  if (length > 0)
    fread(&vec[0], sizeof(Vector3d)*length, 1, file_ptr);
}

void SaveDouble3Vector(vector<Vector3d> &vec, FILE *file_ptr) {
  int		length = static_cast<int> (vec.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&vec[0], sizeof(Vector3d)*length, 1, file_ptr);
}
*/
void LoadString(FILE *file_ptr, wstring &wname) {
	string name;
  int	length;
  fread(&length, sizeof(int), 1, file_ptr);
  name.resize(length);
  if (length > 0)
    fread(&(name)[0], sizeof(char)*length, 1, file_ptr);
  std::wstring wname_tmp(name.begin(), name.end()); // This only works for ASCII filepath
  wname = wname_tmp;
}

void SaveString(const wstring &wname, FILE *file_ptr) {
	string name(wname.begin(), wname.end()); // This only works for ASCII filepath
  int		length = static_cast<int> (name.size());
  fwrite(&length, sizeof(int), 1, file_ptr);
  if (length > 0)
    fwrite(&name[0], sizeof(char)*length, 1, file_ptr);
}

void LoadTopology(FILE *file_ptr, vector<vector<int>> *top) {
  top->clear();
  int len;
  fread(&len, sizeof(int), 1, file_ptr);
  top->resize(len);
  for (int i = 0; i < len; ++i)
    LoadIntegerVector(file_ptr, &(*top)[i]);
}

void SaveTopology(const vector<vector<int>> &top, FILE *file_ptr) {
  int len = static_cast<int> (top.size());
  fwrite(&len, sizeof(int), 1, file_ptr);
  for (int i = 0; i < len; ++i)
    SaveIntegerVector(top[i], file_ptr);
}

void LoadMatrix(FILE *file_ptr, DMatrixF *mat) {
  int n, m;
  fread(&n, sizeof(int), 1, file_ptr);
  fread(&m, sizeof(int), 1, file_ptr);
  if (n > 0 && m > 0) {
    int dim = n*m;
    vector<float> vals;
    vals.resize(dim);
    fread(&vals[0], sizeof(float)*dim, 1, file_ptr);
    mat->SetDimension(m, n);
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        (*mat)[i][j] = vals[i*n + j];
  }
}

void SaveMatrix(const DMatrixF &mat, FILE *file_ptr) {
  int n = mat.GetRowsDim(), m = mat.GetColsDim();
  fwrite(&n, sizeof(int), 1, file_ptr);
  fwrite(&m, sizeof(int), 1, file_ptr);
  if (n > 0 && m > 0) {
    int dim = n*m;
    vector<float> vals;
    vals.resize(dim);
    for (int i = 0; i < m; ++i)
      for (int j = 0; j < n; ++j)
        vals[i*n + j] = mat[i][j];
    fwrite(&vals[0], sizeof(float)*dim, 1, file_ptr);
  }
}