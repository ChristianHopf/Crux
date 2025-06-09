#include "aabb.h"

bool AABB_intersect(struct AABB *a, struct AABB *b){
  return
    (a->min[0] <= b->max[0] && a->max[0] >= b->min[0]) &&
    (a->min[1] <= b->max[1] && a->max[1] >= b->min[1]) &&
    (a->min[2] <= b->max[2] && a->max[2] >= b->min[2]);
}

// Merge b into a
void AABB_merge(struct AABB *a, struct AABB *b){
  a->min[0] = fminf(a->min[0], b->min[0]);
  a->min[1] = fminf(a->min[1], b->min[1]);
  a->min[2] = fminf(a->min[2], b->min[2]);

  a->max[0] = fminf(a->max[0], b->max[0]);
  a->max[1] = fminf(a->max[1], b->max[1]);
  a->max[2] = fminf(a->max[2], b->max[2]);
}

void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex){

}
