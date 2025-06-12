#include "physics/utils.h"

void print_aabb(struct AABB *aabb){
  printf("AABB {\n");
  printf("  min: (%.2f, %.2f, %.2f)\n", aabb->min[0], aabb->min[1], aabb->min[2]);
  printf("  max: (%.2f, %.2f, %.2f)\n", aabb->max[0], aabb->max[1], aabb->max[2]);
  printf("}\n");
}

void print_plane_collider(struct PlaneCollider *plane){
  printf("PlaneCollider:\n");
  printf("{\n");
  printf("  Normal: (%.3f, %.3f, %.3f)\n", plane->normal[0], plane->normal[1], plane->normal[2]);
  printf("  Distance: %.2f\n", plane->distance);
  printf("}\n");
}
