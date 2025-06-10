#include "physics/utils.h"

void print_aabb(struct AABB *aabb){
  printf("AABB {\n");
  printf("  min: (%.2f, %.2f, %.2f)\n", aabb->min[0], aabb->min[1], aabb->min[2]);
  printf("  max: (%.2f, %.2f, %.2f)\n", aabb->max[0], aabb->max[1], aabb->max[2]);
  printf("}\n");
}
