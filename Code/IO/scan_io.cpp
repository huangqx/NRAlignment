#include "scan_io.h"
#include "rply.h"
#include <iostream>
using	namespace	std;

TriangleMesh *cur_mesh = NULL;

static int vertex_cb(p_ply_argument argument) {
  if (cur_mesh != NULL) {
    Vertex &vertex = (*cur_mesh->GetVertexArray())[argument->instance_index];
    if (argument->property->name[0] == 'x') {
      vertex.ori_pos[0] = vertex.cur_pos[0] = static_cast<float> (argument->value);
    }
    if (argument->property->name[0] == 'y') {
      vertex.ori_pos[1] = vertex.cur_pos[1] = static_cast<float> (argument->value);
    }
    if (argument->property->name[0] == 'z') {
      vertex.ori_pos[2] = vertex.cur_pos[2] = static_cast<float> (argument->value);
    }
    if (argument->property->name[0] == 'r') {
      vertex.color[0] = static_cast<float> (argument->value/255.0);
    }
    if (argument->property->name[0] == 'g') {
      vertex.color[1] = static_cast<float> (argument->value/255.0);
    }
    if (argument->property->name[0] == 'b') {
      vertex.color[2] = static_cast<float> (argument->value/255.0);
    }
  }
  return 1;
}

static int face_cb(p_ply_argument argument) {
  if (argument->value_index >= 0) {
    Face &face = (*cur_mesh->GetFaceArray())[argument->instance_index];
    face.vertex_indices[argument->value_index] = static_cast<int> (argument->value);
  }
  return 1;
}

bool Scan_IO::LoadScanFromPLY(const wstring &filepath, TriangleMesh *mesh) {
  mesh->Clear();
  long nvertices, ntriangles;
  CT2CA filen(CString(filepath.c_str()));
  p_ply ply = ply_open(filen, NULL, 0, NULL);
  if (!ply)
    return false;
  if (!ply_read_header(ply))
    return false;
  nvertices = ply_set_read_cb(ply, "vertex", "x", vertex_cb, NULL, 0);
  ply_set_read_cb(ply, "vertex", "y", vertex_cb, NULL, 0);
  ply_set_read_cb(ply, "vertex", "z", vertex_cb, NULL, 1);
  ply_set_read_cb(ply, "vertex", "red", vertex_cb, NULL, 0);
  ply_set_read_cb(ply, "vertex", "green", vertex_cb, NULL, 0);
  ply_set_read_cb(ply, "vertex", "blue", vertex_cb, NULL, 0);
  ntriangles = ply_set_read_cb(ply, "face", "vertex_indices", face_cb, NULL, 0);
  mesh->GetVertexArray()->resize(nvertices);
  mesh->GetFaceArray()->resize(ntriangles);
  cur_mesh = mesh;

  if (!ply_read(ply)) 
    return false;
  ply_close(ply);

  mesh->ComputeOriVertexNormals();
  mesh->OrientNormals();
  mesh->ComputeBoundingBox();
  mesh->SetRenderBit(true);

  //Save filename
  wstring filename;
  ExtractFileName(filepath, filename);
  mesh->SetFileName(filename);
  mesh->SetIsQuadBit(false);

  return true;
}

bool Scan_IO::LoadScanFromOBJ(const wstring &filepath, TriangleMesh *mesh) {
  FILE *file_ptr;
  CT2CA filen(CString(filepath.c_str()));
  fopen_s(&file_ptr, filen, "r");
  if(file_ptr == NULL)
    return false;

  mesh->Clear();
  vector<Vertex> *vertex_array = mesh->GetVertexArray();
  vector<Face> *face_array = mesh->GetFaceArray();
  vector<Vector3f> *texture_corrds = mesh->GetTextureCoords();
  vector<QuadFace> *quad_faces = mesh->GetQuadFaces();
  char str[4096];
  char *result;
  while (true) {
    result = fgets(str, 4096, file_ptr);
    if(result == NULL)
      break;

    if (str[0] == 'v' && str[1] == ' ') {
      Vertex	newV;
      double x, y, z;
      sscanf_s(&str[2], "%lf %lf %lf\n",
        &x, &y, &z);
      newV.ori_pos[0] = float(x);
      newV.ori_pos[1] = float(y);
      newV.ori_pos[2] = float(z);
      newV.cur_pos = newV.ori_pos;
      vertex_array->push_back(newV);
    } else if (str[0] == 'v' && str[1] == 't') {
      Vector3f tc;
      sscanf_s(&str[3], "%f %f %f\n",
        &tc[0], &tc[1], &tc[2]);
      texture_corrds->push_back(tc);
    } else if (str[0] == 'f') {
      Face tri;
      sscanf_s(&str[2], "%ld %ld %ld\n",
        &tri.vertex_indices[0], &tri.vertex_indices[1], &tri.vertex_indices[2]);
      tri.vertex_indices[0]--;
      tri.vertex_indices[1]--;
      tri.vertex_indices[2]--;
      face_array->push_back(tri);
    }
  };

  fclose(file_ptr);
  mesh->ComputeOriVertexNormals();
  mesh->ComputeCurVertexNormals();
  mesh->OrientNormals();
  mesh->ComputeBoundingBox();
  mesh->SetRenderBit(true);
  
  //Save filename
  wstring filename;
  ExtractFileName(filepath, filename);
  mesh->SetFileName(filename);
  mesh->SetIsQuadBit(true);

  return true;
}

bool Scan_IO::SaveScanToOBJ(const TriangleMesh &mesh,
  const Vector3f &global_translation,
  const float &global_scaling,
  const wstring &filepath) {
  if (!mesh.IsQuad())
    return false;

  CT2CA filen(CString(filepath.c_str()));
  FILE *file_ptr;
  fopen_s(&file_ptr, filen, "w");
  if (file_ptr == NULL)
    return false;

  fprintf_s(file_ptr, "# Geomagic Studio\n");
  fprintf_s(file_ptr, "g COORDINATES-0\n");
  const vector<Vertex> *vertex_array = mesh.GetVertexArray();
  const vector<Vector3f> *texture_coords = mesh.GetTextureCoords();
  const vector<QuadFace> *quad_faces = mesh.GetQuadFaces();

  for (unsigned v_id = 0; v_id < vertex_array->size(); ++v_id) {
    const Vertex &v = (*vertex_array)[v_id];
    // Apply the inverse of the global transformation
    Vector3f pos = v.cur_pos/global_scaling + global_translation;
    fprintf_s(file_ptr, "v %lf %lf %lf\n", 
      pos[0], pos[1], pos[2]);
  }
  for (unsigned vt_id = 0; vt_id < texture_coords->size(); ++vt_id) {
    const Vector3f &vt = (*texture_coords)[vt_id];
    fprintf_s(file_ptr, "vt %lf %lf %lf\n", vt[0], vt[1], vt[2]);
  }
  for(unsigned f_id = 0; f_id < quad_faces->size(); ++f_id) {
    const QuadFace &qf = (*quad_faces)[f_id];

    fprintf_s(file_ptr, "f %ld/%ld %ld/%ld %ld/%ld %ld/%ld\n",
      qf.v_ids[0], qf.vt_ids[0],
      qf.v_ids[1], qf.vt_ids[1],
      qf.v_ids[2], qf.vt_ids[2],
      qf.v_ids[3], qf.vt_ids[3]);
  }
  fclose(file_ptr);
  return true;
}

bool Scan_IO::SaveScanToPLY(const TriangleMesh &mesh,
  const Vector3f &global_translation,
  const float &global_scaling,
  const wstring &filepath) {
  if (mesh.IsQuad())
    return false;
  p_ply oply; 
  CT2CA filen(CString(filepath.c_str()));
  oply = ply_create(filen, PLY_LITTLE_ENDIAN, NULL, 0, NULL);
  if (!oply)
    return false;
  if (!ply_add_element(oply, "vertex", mesh.GetVertexArray()->size()))
    return false;
  if (!ply_add_property(oply, "x", PLY_FLOAT, PLY_INT, PLY_INT))
    return false;
  if (!ply_add_property(oply, "y", PLY_FLOAT, PLY_INT, PLY_INT))
    return false;
  if (!ply_add_property(oply, "z", PLY_FLOAT, PLY_INT, PLY_INT))
    return false;
  if (!ply_add_property(oply, "red", PLY_UCHAR, PLY_INT, PLY_INT))
    return false;
  if (!ply_add_property(oply, "green", PLY_UCHAR, PLY_INT, PLY_INT))
    return false;
  if (!ply_add_property(oply, "blue", PLY_UCHAR, PLY_INT, PLY_INT))
    return false;

  if (!ply_add_element(oply, "face", mesh.GetFaceArray()->size()))
    return false;
  if (!ply_add_property(oply, "vertex_indices", PLY_LIST, PLY_UCHAR, PLY_INT))
    return false;
  /* write output header */
  if (!ply_write_header(oply))
    return false;

  for (unsigned v_id = 0; v_id < mesh.GetVertexArray()->size(); ++v_id) {
    const Vertex &v = (*mesh.GetVertexArray())[v_id];
    // Apply the inverse of the global transformation
    Vector3f pos = v.cur_pos/global_scaling + global_translation;
    ply_write(oply, pos[0]);
    ply_write(oply, pos[1]);
    ply_write(oply, pos[2]);
    ply_write(oply, static_cast<int> (255*v.color[0]+0.5));
    ply_write(oply, static_cast<int> (255*v.color[1]+0.5));
    ply_write(oply, static_cast<int> (255*v.color[2]+0.5));
  }

  for(unsigned f_id = 0; f_id < mesh.GetFaceArray()->size(); ++f_id) {
    const Face &face = (*mesh.GetFaceArray())[f_id];
    ply_write(oply, 3);
    ply_write(oply, face.vertex_indices[0]);
    ply_write(oply, face.vertex_indices[1]);
    ply_write(oply, face.vertex_indices[2]);
  }
  if (!ply_close(oply))
    return false;

  return true;
}

bool Scan_IO::SaveReferenceModel(const PointCloud &pc,
  const Vector3f &global_translation,
  const float &global_scaling,
  const wstring &filepath) {
  FILE *file_ptr;
  CT2CA filen(CString(filepath.c_str()));
  fopen_s(&file_ptr, filen, "w");
  if (file_ptr != NULL) {
    const vector<Vertex> *point_array = pc.GetVertexArray();
    for (unsigned point_id = 0;
      point_id < point_array->size();
      ++point_id) {
      Vector3f pos = (*point_array)[point_id].ori_pos;
      // Apply the inverse of the global transformation
      pos = pos/global_scaling + global_translation;
      const Vector3f &nor = (*point_array)[point_id].ori_nor;
      fprintf_s(file_ptr, "%f %f %f %f %f %f %d %d %d\n",
        pos[0], pos[1], pos[2],
        nor[0], nor[1], nor[2],
        static_cast<int> (255*(*point_array)[point_id].color[0]+0.5),
        static_cast<int> (255*(*point_array)[point_id].color[1]+0.5),
        static_cast<int> (255*(*point_array)[point_id].color[2]+0.5));
    }
    fclose(file_ptr);
  }
  return true;
}

void Scan_IO::ExtractFileName(const wstring &filepath, wstring &filename) {
  unsigned dot_pos = filepath.size()-1;
  while (dot_pos > 0) {
    if (filepath[dot_pos] == '.')
      break;
    dot_pos--;
  }
  unsigned  folder_pos = dot_pos-1;
  while (folder_pos > 0) {
    if (filepath[folder_pos] == '\\')
      break;
    folder_pos--;
  }

  filename.resize(dot_pos - folder_pos-1);
  for (unsigned i = 0; i < dot_pos - folder_pos-1; ++i)
    filename[i] = filepath[i+folder_pos+1];
}