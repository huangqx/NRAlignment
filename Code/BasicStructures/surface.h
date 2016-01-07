#ifndef surface_h_
#define surface_h_

#include <string>

#include "bounding_box.h"
#include "linear_algebra.h"
#include "linear_algebra_templcode.h"

using namespace std;

const unsigned int SURFACE_RENDER_BIT = 1<<13;
const unsigned int SURFACE_LOCKED_BIT = 1<<14;
const unsigned int SURFACE_ISQUAD_BIT = 1<<15;

class Surface {
 public:
  Surface();
  virtual ~Surface();
  bool Render() const;
  void SetRenderBit(bool f);
  bool Locked() const;
  void SetLockedBit(bool f);
  bool IsQuad() const;
  void SetIsQuadBit(bool f);
  
  BoundingBox* GetBoundingBox() {
    return &bounding_box_;
  }
  const BoundingBox* GetBoundingBox() const {
    return &bounding_box_;
  }
  void SetFileName(wstring &name) {
    filename = name;
  }
  wstring &GetFileName() {
    return filename;
  }
  const wstring &GetFileName() const {
    return filename;
  }
 protected:
  wstring filename;
  BoundingBox	bounding_box_; // Bounding box of the current surface
 private:
  int flags_; 
};
#endif