#ifndef PHYSICS_AABB_H
#define PHYSICS_AABB_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include "collider.h"
#include "shader.h"

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
  GLuint VAO, VBO, EBO;
};


void AABB_merge(struct AABB *a, struct AABB *b, struct AABB *dest);
void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, struct AABB *dest);
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex);

// Collision tests
bool AABB_intersect_AABB(struct AABB *a, struct AABB *b);
bool AABB_intersect_plane(struct AABB *box, struct PlaneCollider *plane);

void AABB_init(struct AABB *aabb);
void AABB_render(struct AABB *aabb, mat4 model, mat4 view, mat4 projection);

#endif
