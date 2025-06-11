#ifndef PHYSICS_AABB_H
#define PHYSICS_AABB_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include "shader.h"

struct AABB {
  vec3 min;
  vec3 max;

  GLuint VAO, VBO, EBO;
};


bool AABB_intersect(struct AABB *a, struct AABB *b);
void AABB_merge(struct AABB *a, struct AABB *b);
void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, struct AABB *dest);
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex);

void AABB_init(struct AABB *aabb);
void AABB_render(struct AABB *aabb, mat4 model, mat4 view, mat4 projection);

#endif
