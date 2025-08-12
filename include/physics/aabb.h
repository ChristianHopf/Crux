#pragma once

#include <cglm/cglm.h>
#include <stdbool.h>
#include "plane.h"

// struct AABB {
//   vec3 min;
//   vec3 max;
//
//   GLuint VAO, VBO, EBO;
// };

struct AABB {
  vec3 center;
  vec3 extents;

  // Necessary for recursively building an AABB
  // that bounds a model, given Assimp's node hierarchy
  bool initialized;
  // GLuint VAO, VBO, EBO;
};


void AABB_merge(struct AABB *a, struct AABB *b);
void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, vec3 scale, struct AABB *dest);
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex);

// Collision tests
bool AABB_intersect_AABB(struct AABB *a, struct AABB *b);
bool AABB_intersect_plane(struct AABB *box, struct Plane *plane);
