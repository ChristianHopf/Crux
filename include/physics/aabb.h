#ifndef AABB_H
#define AABB_H

#include <cglm/cglm.h>
#include <stdbool.h>

struct AABB {
  vec3 min;
  vec3 max;
};

bool AABB_intersect(struct AABB *a, struct AABB *b){
  return
    (a->min[0] <= b->max[0] && a->max[0] >= b->min[0]) &&
    (a->min[1] <= b->max[1] && a->max[1] >= b->min[1]) &&
    (a->min[2] <= b->max[2] && a->max[2] >= b->min[2]);
}

#endif
