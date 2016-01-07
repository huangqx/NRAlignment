#ifndef display_h_
#define display_h_

#include "camera_container.h"
#include "light_container.h"
#include "color_container.h"
#include "data_container.h"

#define PICKRANGE 4    
#define HALFPICKRANGE 2
#define VIEW_TRIANGULAR_MESH 30
#define VIEW_POINTCLOUD 31

class DataContainer;
class Display3Dim {
 public:
	Display3Dim();
  ~Display3Dim();

  void Init();
  void Draw();
  void RenderData();
  void SetData(DataContainer *data);
  CameraContainer *GetCamera() {
    return &camera_container_;
  }
  ColorContainer *GetColor() {
    return &color_container_;
  }
  LightContainer *GetLight() {
    return &light_container_;
  }
  void MouseMove(int nFlags, int cx, int cy);
  void Reshape(int cx, int cy);

  bool CheckRenderingModeMesh() { return mesh_mode_; }
  bool CheckRenderingModePointCloud() { return !mesh_mode_; }
  
  bool UsingSmoothShading()	{ return smooth_shading_; }
  bool UsingAntialiasing() { return antialiasing_; }
  bool UsingCulling() { return culling_; }
  bool UsingLighting() { return lighting_; }

 
  bool CheckViewTexture() { return view_texture_; }
  bool CheckViewAligningError() { return view_aligning_error_; }
  bool CheckViewOriginalSetting() { return view_original_setting_; }

  void UpdateRenderingMode() { mesh_mode_ = !mesh_mode_; }

  void UpdateSmoothShadingFlag() { smooth_shading_ = !smooth_shading_; }
  void UpdateAntialiasingFlag() { antialiasing_ = !antialiasing_; }
  void UpdateCullingFlag() { culling_ = !culling_; }
  void UpdateLightingFlag() { lighting_ = !lighting_; }

  void UpdateViewAligningError() {
    view_aligning_error_ = !view_aligning_error_;
    if (view_aligning_error_) {
      view_texture_ = false;
    }
  }
  void UpdateViewOriginalSetting() { view_original_setting_ = !view_original_setting_; }

  void UpdateViewTexture() {
    view_texture_ = !view_texture_;
    if (view_texture_) {
      view_aligning_error_ = false;
    }
  }
	DataContainer *data_pt;
 private:
  // Triangle mesh
  void DrawMesh(const TriangleMesh &mesh);

  // Point cloud
  void DrawPointCloud(const PointCloud &pc);

  // Reference surface
  void DrawReferenceSurface(const PointCloud &pc);

  /// Camera
  CameraContainer	camera_container_;

  /// Manipulate the color and material
  ColorContainer color_container_;

  /// The lights
  LightContainer light_container_;

  bool lighting_;
  bool antialiasing_;
  bool culling_;
  bool smooth_shading_;

  bool view_aligning_error_; // Show the aligning error as a function 
  bool view_texture_; // Show texture information

	bool mesh_mode_; // Display each scan as a mesh or a point cloud

  bool view_original_setting_; //View registered scans
};

#endif