#include "stdafx.h"
#include "display.h"
#include "primitive_rendering.h"
#include "data_container.h"

Display3Dim::Display3Dim() {
  lighting_ = true;
  antialiasing_	= true;
  culling_ = true;
  smooth_shading_	= true;

  view_aligning_error_ = false;
  view_texture_ = false;

  mesh_mode_ = false;

  view_original_setting_ = false;
}

Display3Dim::~Display3Dim() {
}

void Display3Dim::SetData(DataContainer *data) {
  data_pt = data;
}

void Display3Dim::Init() {
  // Default mode
  glEnable(GL_DEPTH_TEST);
  glClearColor(color_container_.back_color[0],
    color_container_.back_color[1],
    color_container_.back_color[2],
    1.0f);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  int	nLights = int(light_container_.lights.size());
  for (int lId = 0; lId < nLights; lId++) {
    Light3Dim	&light = light_container_.lights[lId];
    if (lId == 0) {
      glLightfv(GL_LIGHT0, GL_SPECULAR, light.specular);
      glLightfv(GL_LIGHT0, GL_AMBIENT,  light.ambient);
      glLightfv(GL_LIGHT0, GL_POSITION, light.position);
      glLightfv(GL_LIGHT0, GL_DIFFUSE,  light.diffuse);
      } else if(lId == 1) {
        glLightfv(GL_LIGHT1, GL_SPECULAR, light.specular);
        glLightfv(GL_LIGHT1, GL_AMBIENT,  light.ambient);
        glLightfv(GL_LIGHT1, GL_POSITION, light.position);
        glLightfv(GL_LIGHT1, GL_DIFFUSE,  light.diffuse);
      }
    }
  // lighting
  glLightModelf(GL_LIGHT_MODEL_TWO_SIDE, 10.0f);
  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_AUTO_NORMAL);

  // back material
  float MatAmbientBack[]  = {0.0f, 1.0f, 0.0f, 1.0f};
  glMaterialfv(GL_BACK,GL_AMBIENT,MatAmbientBack);
}

void Display3Dim::Draw() {
//  Init();
  if (data_pt->first_view_) {
    camera_container_.Init(*data_pt->GetBoundingBox());
    data_pt->first_view_ = false;
  }
  camera_container_.Setup();
  glClearColor(color_container_.back_color[0], 
    color_container_.back_color[1], 
    color_container_.back_color[2],
    1.f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if(smooth_shading_)
    glShadeModel(GL_SMOOTH);
  else
    glShadeModel(GL_FLAT);

  // culling option
  if(culling_)
    glEnable(GL_CULL_FACE);
  else
    glDisable(GL_CULL_FACE);

  // polygon mode (point, line or fill)
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // antialiasing
  if(antialiasing_) {
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glLineWidth(1.5f);
    } else {
      glDisable(GL_LINE_SMOOTH);
      glDisable(GL_BLEND);
      glLineWidth(1.0f);
    }

  // drawing
  glPushMatrix();

  // lighting option
  if(lighting_) {
    glEnable(GL_LIGHTING);
  } else {
    glDisable(GL_LIGHTING);
  }

	RenderData();

  glDisable(GL_LIGHTING);
  glDisable(GL_POLYGON_OFFSET_LINE);
  glPushMatrix();
  double d = sqrt(camera_container_.eye_point_pos.getSqrNorm())/30.0;
  glScaled(d,d,d);
  glPopMatrix();
}

void Display3Dim::RenderData() {
  if(data_pt == NULL)
    return;

  // Shapes 
  vector<ShapeContainer> *shapes = data_pt->GetInputShapes();
  for (unsigned shape_id = 0; shape_id < shapes->size(); ++shape_id) {
    if (shape_id %6 == 0) {
      color_container_.ChangeMaterial(MATERIAL_SILVER, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    } else if (shape_id %6 == 1) {
      color_container_.ChangeMaterial(MATERIAL_GOLD, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    } else if (shape_id %6 == 2) {
      color_container_.ChangeMaterial(MATERIAL_LIGHT_BLUE, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    } else if (shape_id %6 == 3) {
      color_container_.ChangeMaterial(MATERIAL_RUDY, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    } else if (shape_id %6 == 4){
      color_container_.ChangeMaterial(MATERIAL_GRASS, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    } else {
      color_container_.ChangeMaterial(MATERIAL_TURQUOISE, true);
      color_container_.ChangeMaterial(MATERIAL_JADE, false);
    }
    if (!mesh_mode_)
      DrawPointCloud((*shapes)[shape_id].surface);
    else
      DrawMesh((*shapes)[shape_id].surface);
  }
  DrawPointCloud(*data_pt->GetReferenceModel());
}

void Display3Dim::DrawMesh(const TriangleMesh &mesh) {
  if (!mesh.Render())
    return;

  if (view_texture_ || view_aligning_error_)
    color_container_.EnableColorMaterial();

  const vector<Vertex> *vertex_array = mesh.GetVertexArray();
  const vector<Face> *face_array = mesh.GetFaceArray();
  for(unsigned face_id = 0; face_id < face_array->size(); ++face_id) {
    const Face &face = (*face_array)[face_id];
    ::glBegin(GL_POLYGON);
    if (!smooth_shading_) {
      ::glNormal3d(face.cur_nor[0], face.cur_nor[1], face.cur_nor[2]);
    }

    for (int i = 0; i < 3; ++i) {
      const Vertex &v = (*vertex_array)[face.vertex_indices[i]];
      if (view_texture_)
        glColor3f(v.color[0], v.color[1], v.color[2]);
      
      if (view_aligning_error_)
        color_container_.SetColor(v.iso_value);
      
      if (smooth_shading_){
        if (!view_original_setting_)
          ::glNormal3d(v.cur_nor[0], v.cur_nor[1], v.cur_nor[2]);
        else
          ::glNormal3d(v.ori_nor[0], v.ori_nor[1], v.ori_nor[2]);
      }
      if (!view_original_setting_)
        ::glVertex3d(v.cur_pos[0], v.cur_pos[1], v.cur_pos[2]);
      else
        ::glVertex3d(v.ori_pos[0], v.ori_pos[1], v.ori_pos[2]);
    }
    ::glEnd(); // end polygon assembly
  }
  if (view_texture_ || view_aligning_error_)
    color_container_.DisableColorMaterial();
}

void Display3Dim::DrawPointCloud(const PointCloud &pc) {
  if (!pc.Render())
    return;

  const vector<Vertex> *point_array = pc.GetVertexArray();
  if (view_texture_ || view_aligning_error_)
    color_container_.EnableColorMaterial();

  glPointSize(5.f);
  glBegin(GL_POINTS);
  for (unsigned point_id = 0; point_id < point_array->size(); ++point_id) {
    const Vertex &vertex = (*point_array)[point_id];
    if (view_texture_) 
      glColor3f(vertex.color[0], vertex.color[1], vertex.color[2]);
    
    if (view_aligning_error_)
      color_container_.SetColor(vertex.iso_value);
    
    if (!view_original_setting_) {
      glNormal3f(vertex.cur_nor[0], vertex.cur_nor[1], vertex.cur_nor[2]);
      glVertex3f(vertex.cur_pos[0], vertex.cur_pos[1], vertex.cur_pos[2]);
    } else {
      glNormal3f(vertex.ori_nor[0], vertex.ori_nor[1], vertex.ori_nor[2]);
      glVertex3f(vertex.ori_pos[0], vertex.ori_pos[1], vertex.ori_pos[2]);
    }
  }
  glEnd();
  if (view_texture_ || view_aligning_error_)
    color_container_.DisableColorMaterial();
}

void Display3Dim::Reshape(int cx, int cy) {
  camera_container_.Reshape(cx, cy);
}

void Display3Dim::MouseMove(int nFlags, int cx, int cy) {
  camera_container_.MouseMove(nFlags, cx, cy);
}