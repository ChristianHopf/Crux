#ifndef PHYSICS_AABB_H
#define PHYSICS_AABB_H

#include <cglm/cglm.h>
#include <stdbool.h>

struct AABB {
  vec3 min;
  vec3 max;
};

bool AABB_intersect(struct AABB *a, struct AABB *b);
void AABB_merge(struct AABB *a, struct AABB *b);
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex);

#endif
