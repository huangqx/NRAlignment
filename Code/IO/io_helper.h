#ifndef io_helper_h_
#define io_helper_h_

#include <string>
#include <vector>
using namespace std;

#include "dynamic_linear_algebra_templcode.h"
#include "dynamic_linear_algebra.h"
#include "linear_algebra_templcode.h"
#include "linear_algebra.h"

void LoadIntegerVector(FILE *file_ptr, vector<int> *vec);
void SaveIntegerVector(const vector<int> &vec, FILE *file_ptr);

void LoadTopology(FILE *file_ptr, vector<vector<int>> *top);
void SaveTopology(const vector<vector<int>> &top, FILE *file_ptr);

void LoadMatrix(FILE *file_ptr, DMatrixF *mat);
void SaveMatrix(const DMatrixF &mat, FILE *file_ptr);

void LoadString(FILE *file_ptr, wstring &name);
void SaveString(const wstring &name, FILE *file_ptr);

#endif