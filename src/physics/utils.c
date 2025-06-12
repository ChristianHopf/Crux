#include "physics/utils.h"

void print_aabb(struct AABB *aabb){
  // Convert AABB to min/max format for debug printing
  vec3 min;
  vec3 max;
  
  for (int i = 0; i < 3; i++){
    min[i] = aabb->center[i] - aabb->extents[i];
    max[i] = aabb->center[i] + aabb->extents[i];
  }

  printf("AABB {\n");
  printf("  min: (%.2f, %.2f, %.2f\n)", min[0], min[1], min[2]);
  printf("  max: (%.2f, %.2f, %.2f\n)", max[0], max[1], max[2]);
  printf("}\n");
}

void print_plane_collider(struct PlaneCollider *plane){
  printf("PlaneCollider:\n");
  printf("{\n");
  printf("  Normal: (%.3f, %.3f, %.3f)\n", plane->normal[0], plane->normal[1], plane->normal[2]);
  printf("  Distance: %.2f\n", plane->distance);
  printf("}\n");
}
