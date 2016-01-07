#include "patch_container.h"
#include "io_helper.h"

void SurfacePatch::Read(FILE *file_ptr) {
  fread(&motion[0], sizeof(Affine3d), 1, file_ptr);
  LoadIntegerVector(file_ptr, &kernel_region_indices);
  int num_points;
  fread(&num_points, sizeof(int), 1, file_ptr);
  points.clear();
  points.resize(num_points);
  if (num_points > 0)
    fread(&points[0], sizeof(AssociWeight)*num_points, 1, file_ptr);
}

void SurfacePatch::Write(FILE *file_ptr) {
  fwrite(&motion[0], sizeof(Affine3d), 1, file_ptr);
  SaveIntegerVector(kernel_region_indices, file_ptr);
  int num_points = static_cast<int> (points.size());
  fwrite(&num_points, sizeof(int), 1, file_ptr);
  if (num_points > 0)
    fwrite(&points[0], sizeof(AssociWeight)*num_points, 1, file_ptr);
}

void PatchContainer::Read(FILE *file_ptr) {
  int num_patches;
  fread(&num_patches, sizeof(int), 1, file_ptr);
  surface_patches.clear();
  surface_patches.resize(num_patches);
  for (int patch_id = 0; patch_id < num_patches; ++patch_id) {
    surface_patches[patch_id].Read(file_ptr);
  }
}

void PatchContainer::Write(FILE *file_ptr) {
  int num_patches = static_cast<int> (surface_patches.size());
  fwrite(&num_patches, sizeof(int), 1, file_ptr);
  for (int patch_id = 0; patch_id < num_patches; ++patch_id) {
    surface_patches[patch_id].Write(file_ptr);
  }
}