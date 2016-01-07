#include <algorithm>
using namespace std;

#include "data_container.h"
#include "color_container.h"
#include "OptionView.h"
#include "io_helper.h"

void DataContainer::Read(FILE *file_ptr){
  int num_of_shapes;
  fread(&num_of_shapes, sizeof(int), 1, file_ptr);
  input_shapes_.clear();
  input_shapes_.resize(num_of_shapes);
  for (int shape_id = 0; shape_id < num_of_shapes; ++shape_id)
    input_shapes_[shape_id].Read(file_ptr);
  reference_model_.Read(file_ptr);
  bounding_box_.Read(file_ptr);
  fread(&global_translation_, sizeof(Vector3f), 1, file_ptr);
  fread(&global_scaling_, sizeof(float), 1, file_ptr);
}

void DataContainer::Write(FILE *file_ptr){
  int num_of_shapes = static_cast<int> (input_shapes_.size());
  fwrite(&num_of_shapes, sizeof(int), 1, file_ptr);
  for (int shape_id = 0; shape_id < num_of_shapes; ++shape_id) {
    input_shapes_[shape_id].Write(file_ptr);
  }
  reference_model_.Write(file_ptr);
  bounding_box_.Write(file_ptr);
  fwrite(&global_translation_, sizeof(Vector3f), 1, file_ptr);
  fwrite(&global_scaling_, sizeof(float), 1, file_ptr);
}

void ShapeContainer::Read(FILE *file_ptr) {
  surface.Read(file_ptr);
//  patches.Read(file_ptr);
//  opt_struct.Read(file_ptr);
}

void ShapeContainer::Write(FILE *file_ptr) {
  surface.Write(file_ptr);
//  patches.Write(file_ptr);
//  opt_struct.Write(file_ptr);
}

void DataContainer::ClearRenderingFlags() {
  for (unsigned shape_id = 0;
    shape_id < input_shapes_.size();
    ++shape_id) {
    input_shapes_[shape_id].surface.SetRenderBit(false);
  }
  reference_model_.SetRenderBit(false);
}

bool DataContainer::SetRenderingFlags(const CString &itemName) {
  if (itemName.Left(7) == "RefSurf") {
    reference_model_.SetRenderBit(true);
  } else if (itemName.Left(6) == "Shapes") {
    for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
      input_shapes_[shape_id].surface.SetRenderBit(true);
    }
  } else {
    wstring name(itemName);
    for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
      if (input_shapes_[shape_id].surface.GetFileName() == name) {
        input_shapes_[shape_id].surface.SetRenderBit(true);
        break;
      }
    }
  }
  return false;
}

bool DataContainer::CanLockScan(CString itemName) {
  wstring name = itemName.GetBuffer();
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.GetFileName() == name) {
      return !input_shapes_[shape_id].surface.Locked();
    }
  }
  return false;
}

bool DataContainer::CanUnLockScan(CString itemName) {
  wstring name = itemName.GetBuffer();
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.GetFileName() == name) {
      return input_shapes_[shape_id].surface.Locked();
    }
  }
  return false;
}

bool DataContainer::LockScan(CString itemName) {
  wstring name = itemName.GetBuffer();
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.GetFileName() == name) {
      input_shapes_[shape_id].surface.SetLockedBit(true);
      return true;
    }
  }
  return false;
}

bool DataContainer::UnLockScan(CString itemName) {
  wstring name = itemName.GetBuffer();
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.GetFileName() == name) {
      input_shapes_[shape_id].surface.SetLockedBit(false);
      return true;
    }
  }
  return false;
}

void DataContainer::LockSelected() {
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.Render())
      input_shapes_[shape_id].surface.SetLockedBit(true);
  }
}

void DataContainer::UnLockSelected() {
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.Render())
      input_shapes_[shape_id].surface.SetLockedBit(false);
  }
}

bool DataContainer::DeleteScan(CString itemName) {
  wstring name = itemName.GetBuffer();
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (input_shapes_[shape_id].surface.GetFileName() == name) {
      for (unsigned i = shape_id+1; i < input_shapes_.size(); ++i)
        input_shapes_[i-1] = input_shapes_[i];

      input_shapes_.resize(input_shapes_.size()-1);
      return true;
    }
  }
  return false;
}

void DataContainer::DeleteSelected() {
  unsigned num_remaining = 0;
  for (unsigned shape_id = 0; shape_id < input_shapes_.size(); ++shape_id) {
    if (!input_shapes_[shape_id].surface.Render()) {
      input_shapes_[num_remaining] = input_shapes_[shape_id];
      num_remaining++;
    }
  }
  input_shapes_.resize(num_remaining);
}

void DataContainer::SetNames(COptionView *view) {
  SetShapeNames(view);
}

void DataContainer::SetShapeNames(COptionView *view) {
  view->m_ctrlTree.DeleteBranch(view->root_shapes);
//  CString name;
  for (unsigned shape_id = 0;
    shape_id < input_shapes_.size();
    ++shape_id) {
//    name.Format("shape_%d", shape_id);
    CString name(input_shapes_[shape_id].surface.GetFileName().c_str());
    if (input_shapes_[shape_id].surface.Locked())
      view->m_ctrlTree.InsertItem(name, 1, 1, view->root_shapes);
    else
      view->m_ctrlTree.InsertItem(name, 2, 2, view->root_shapes);
  }
  view->Invalidate();
}

void DataContainer::Normalize() {
  ComputeBoundingBox();
  for (int i = 0; i < 3; ++i)
    global_translation_[i] = static_cast<float> (
    bounding_box_.center_point[i]);
  global_scaling_ = 0.01f;

  for (unsigned shape_index = 0;
    shape_index < input_shapes_.size();
    ++shape_index) {
    ShapeContainer &shape = input_shapes_[shape_index];
    for (unsigned vertex_index = 0;
      vertex_index < shape.surface.GetVertexArray()->size();
      ++vertex_index) {
      Vertex &vertex = (*shape.surface.GetVertexArray())[vertex_index];
      vertex.ori_pos -= global_translation_;
      vertex.ori_pos *= global_scaling_;
      vertex.cur_pos = vertex.ori_pos;
    }
    shape.surface.ComputeBoundingBox();
  }
  ComputeBoundingBox();
}

void DataContainer::ComputeBoundingBox() {
  bounding_box_.Initialize();
  for (unsigned shape_index = 0; shape_index < input_shapes_.size(); ++shape_index) {
    bounding_box_.Insert_A_Box(input_shapes_[shape_index].surface.GetBoundingBox());
  }
  printf("%f %f %f\n", bounding_box_.center_point[0],
    bounding_box_.center_point[1],
    bounding_box_.center_point[2]);
  first_view_ = true;
}

void ShapeContainer::GenerateCurrentSurface(const PatchContainer &pc,
  const bool &blend) {
  if (blend) {
    int num_points = static_cast<int> (surface.GetVertexArray()->size());
    vector<Vector3f> cur_pos, cur_nor;
    vector<float> weights;
    cur_pos.resize(num_points);
    cur_nor.resize(num_points);
    weights.resize(num_points);

    for (int pt_id = 0; pt_id < num_points; ++pt_id) {
      for (int i = 0; i < 3; ++i) {
        cur_pos[pt_id][i] = 0.f;
        cur_nor[pt_id][i] = 0.f;
      }
      weights[pt_id] = 0.0;
    }

    for (unsigned patch_id = 0;
      patch_id < pc.surface_patches.size();
      ++patch_id) {
      const SurfacePatch &sp = pc.surface_patches[patch_id];
      for (unsigned i = 0; i < sp.points.size(); ++i) {
        const AssociWeight &aw = sp.points[i];
        int pt_id = aw.vertex_index;
        const Vector3f &pos = (*surface.GetVertexArray())[pt_id].ori_pos;
        const Vector3f &nor = (*surface.GetVertexArray())[pt_id].ori_nor;
        Vector3d p = sp.motion[0] + sp.motion[1]*pos[0] + sp.motion[2]*pos[1] + sp.motion[3]*pos[2];
        Vector3d n = sp.motion[1]*nor[0] + sp.motion[2]*nor[1] + sp.motion[3]*nor[2];
        weights[pt_id] += aw.weight;
        for (int k = 0; k < 3; ++k) {
          cur_pos[pt_id][k] += static_cast<float> (aw.weight*p[k]);
          cur_nor[pt_id][k] += static_cast<float> (aw.weight*n[k]);
        }
      }
    }

    for (int pt_id = 0; pt_id < num_points; ++pt_id) {
      if (weights[pt_id] > 1e-3f) {
        (*surface.GetVertexArray())[pt_id].cur_pos = cur_pos[pt_id]/weights[pt_id];
        (*surface.GetVertexArray())[pt_id].cur_nor = cur_nor[pt_id]/weights[pt_id];
      }
    }
  } else {
    for (unsigned patch_id = 0;
      patch_id < pc.surface_patches.size();
      ++patch_id) {
      const SurfacePatch &patch = pc.surface_patches[patch_id];
      const Affine3d &aff = patch.motion;
      for (unsigned i = 0; i < patch.kernel_region_indices.size(); ++i) {
        int vertex_index = patch.kernel_region_indices[i];
        Vertex &vertex = (*surface.GetVertexArray())[vertex_index];
        Vector3d transformed_pos = aff[1]*vertex.ori_pos[0]
        + aff[2]*vertex.ori_pos[1] + aff[3]*vertex.ori_pos[2] + aff[0];
        Vector3d transformed_nor = aff[1]*vertex.ori_nor[0]
        + aff[2]*vertex.ori_nor[1] + aff[3]*vertex.ori_nor[2];

        for (int k = 0; k < 3; ++k) {
          vertex.cur_pos[k] = static_cast<float> (transformed_pos[k]);
          vertex.cur_nor[k] = static_cast<float> (transformed_nor[k]);
        }
      }
    }
  }
}

void ShapeContainer::ApplyGlobalTransform(const Vector3f &global_translation,
  const float &global_scaling) {
  for (unsigned v_id = 0; v_id < surface.GetVertexArray()->size(); ++v_id) {
    Vertex &vertex = (*surface.GetVertexArray())[v_id];
    vertex.ori_pos -= global_translation;
    vertex.ori_pos *= global_scaling;
    vertex.cur_pos = vertex.ori_pos;
  }
  surface.ComputeBoundingBox();
}