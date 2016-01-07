#ifndef scan_io_h_
#define scan_io_h_

#include <string>
#include "data_container.h"

using namespace std;

class Scan_IO {
 public:
	Scan_IO(){
  }
	~Scan_IO(){
  }

  bool LoadScanFromPLY(const wstring &filepath, TriangleMesh *mesh);
  bool LoadScanFromOBJ(const wstring &filepath, TriangleMesh *mesh);
  bool SaveScanToOBJ(const TriangleMesh &mesh,
    const Vector3f &global_translation,
    const float &global_scaling,
    const wstring &filepath);
  bool SaveScanToPLY(const TriangleMesh &mesh,
    const Vector3f &global_translation,
    const float &global_scaling,
    const wstring &filepath);
  bool SaveReferenceModel(const PointCloud &pc,
    const Vector3f &global_translation,
    const float &global_scaling,
    const wstring &filepath);
  void ExtractFileName(const wstring &filepath, wstring &filename);
};

#endif