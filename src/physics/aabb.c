#include "physics/aabb.h"
#include "physics/utils.h"
#include "utils.h"
#include <cglm/util.h>
#include <cglm/vec3.h>

// Shader program for rendering AABB wireframes
static Shader *aabbShader;


// Merge b into a
void AABB_merge(struct AABB *a, struct AABB *b){
  // a->min[0] = fminf(a->min[0], b->min[0]);
  // a->min[1] = fminf(a->min[1], b->min[1]);
  // a->min[2] = fminf(a->min[2], b->min[2]);
  //
  // a->max[0] = fmaxf(a->max[0], b->max[0]);
  // a->max[1] = fmaxf(a->max[1], b->max[1]);
  // a->max[2] = fmaxf(a->max[2], b->max[2]);

  // Compute new center
  // a->center[0] = (a->center[0] + b->center[0]) * 0.5f;
  // a->center[1] = (a->center[1] + b->center[1]) * 0.5f;
  // a->center[2] = (a->center[2] + b->center[2]) * 0.5f;

  // Compute new radius
  // a->radius[0] = (a->radius[0] + b->radius[0]);
  // a->radius[1] = (a->radius[1] + b->radius[1]);
  // a->radius[2] = (a->radius[2] + b->radius[2]);

  // If a is not initialized, copy b into a.
  // If b is not initialized, return.
  if (!a->initialized){
    glm_vec3_copy(b->center, a->center);
    glm_vec3_copy(b->extents, a->extents);
    a->initialized = true;
    return;
  }
  if (!b->initialized){
    return;
  }

  vec3 minA, maxA;
  glm_vec3_sub(a->center, a->extents, minA);
  glm_vec3_add(a->center, a->extents, maxA);

  vec3 minB, maxB;
  glm_vec3_sub(b->center, b->extents, minB);
  glm_vec3_add(b->center, b->extents, maxB);

  vec3 minMerged;
  minMerged[0] = glm_min(minA[0], minB[0]);
  minMerged[1] = glm_min(minA[1], minB[1]);
  minMerged[2] = glm_min(minA[2], minB[2]);

  vec3 maxMerged;
  maxMerged[0] = glm_max(maxA[0], maxB[0]);
  maxMerged[1] = glm_max(maxA[1], maxB[1]);
  maxMerged[2] = glm_max(maxA[2], maxB[2]);

  vec3 centerMerged, extentsMerged;
  glm_vec3_add(minMerged, maxMerged, centerMerged);
  glm_vec3_scale(centerMerged, 0.5f, centerMerged);
  glm_vec3_sub(maxMerged, minMerged, extentsMerged);
  glm_vec3_scale(extentsMerged, 0.5f, extentsMerged);

  glm_vec3_copy(centerMerged, a->center);
  glm_vec3_copy(extentsMerged, a->extents);

  a->initialized = true;

  return;
}

void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, vec3 scale, struct AABB *dest){
  // printf("Time to update AABB src and store in dest. Current values of src and dest:\n");
  // print_aabb(src);
  // print_aabb(dest);
  // For all three axes:
  // for (int i = 0; i < 3; i++){
  //   // Add translation
  //   dest->min[i] = dest->max[i] = translation[i];
  //
  //   // Form the min and max extents of this axis by summing smaller and larger terms
  //   for (int j = 0; j < 3; j++){
  //     float e = rotation[i][j] * src->min[j];
  //     float f = rotation[i][j] * src->max[j];
  //     if (e < f){
  //       dest->min[i] += e;
  //       dest->max[i] += f;
  //     } else{
  //       dest->min[i] += f;
  //       dest->max[i] += e;
  //     }
  //   }
  // }

  // Scale center properly if not zero in model space

  for (int i = 0; i < 3; i++){
    dest->center[i] = translation[i];
    dest->extents[i] = 0.0f;
    for (int j = 0; j < 3; j++){
      dest->center[i] += rotation[i][j] * src->center[j];
      dest->extents[i] += fabs(rotation[i][j]) * src->extents[j];
    }
    dest->extents[i] *= fabs(scale[i]);
  }
  glm_vec3_mul(dest->center, scale, dest->center);
}

//Figure out an optimal algorithm for this later.
// If you update the minimum x component, for example,
// you certainly don't have to update the maximum x component.
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex){
  // Minimum
  // aabb->min[0] = fminf(aabb->min[0], vertex[0]);
  // aabb->min[1] = fminf(aabb->min[1], vertex[1]);
  // aabb->min[2] = fminf(aabb->min[2], vertex[2]);

  // Maximum
  // aabb->max[0] = fmaxf(aabb->max[0], vertex[0]);
  // aabb->max[1] = fmaxf(aabb->max[1], vertex[1]);
  // aabb->max[2] = fmaxf(aabb->max[2], vertex[2]);

  // For each axis, if the vertex extends the AABB in either direction,
  // update the center and radius on that axis by half that distance
  for (int i = 0; i < 3; i++){
    if (vertex[i] > (aabb->center[i] + aabb->extents[i])){
      float dist = vertex[i] - (aabb->center[i] + aabb->extents[i]);
      aabb->center[i] += dist * 0.5f;
      aabb->extents[i] += dist * 0.5f;
    }
    if (vertex[i] < (aabb->center[i] - aabb->extents[i])){
      float dist = aabb->center[i] - aabb->extents[i] - vertex[i];
      aabb->center[i] -= dist * 0.5f;
      aabb->extents[i] += dist * 0.5f;
    }
  }
}

// COLLISION TESTS
//
// Intersection between AABBs
bool AABB_intersect_AABB(struct AABB *a, struct AABB *b){
  // if (a->max[0] < b->min[0] || a->min[0] > b->max[0]) return false;
  // if (a->max[2] < b->min[2] || a->min[2] > b->max[2]) return false;
  // if (a->max[1] < b->min[1] || a->min[1] > b->max[1]) return false;
  // return true;

  if (fabs(a->center[0] - b->center[0]) > (a->extents[0] + b->extents[0])) return false;
  if (fabs(a->center[1] - b->center[1]) > (a->extents[1] + b->extents[1])) return false;
  if (fabs(a->center[2] - b->center[2]) > (a->extents[2] + b->extents[2])) return false;
  return true;
}

// Intersection between an AABB and a plane
bool AABB_intersect_plane(struct AABB *box, struct PlaneCollider *plane){

  // Get radius of the extents' projection interval onto the plane's normal
  float r =
    box->extents[0] * fabs(plane->normal[0]) +
    box->extents[1] * fabs(plane->normal[1]) +
    box->extents[2] * fabs(plane->normal[2]); 
  // Get distance from the center of AABB to the plane
  float s = glm_dot(plane->normal, box->center) - plane->distance;

  // If the distance from the center to the plane is within the interval,
  // the AABB is colliding with the plane
  return fabs(s) <= r;
}

// RENDERING FUNCTIONS
//
// Assumes a model's AABB will never change
void AABB_init(struct AABB *aabb){
  // Create the AABB shader
  if (!aabbShader){
    Shader *aabbShaderPtr = shader_create("shaders/physics/aabb.vs", "shaders/physics/aabb.fs");
    if (!aabbShaderPtr){
      printf("Error: failed to create AABB shader\n");
      return;
    }
    aabbShader = aabbShaderPtr;
  }

  // Define vertices and indices for the box, based on min and max
  float vertices[24] = {
    aabb->center[0] + aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] + aabb->extents[2],

    aabb->center[0] - aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] - aabb->extents[2],

    // aabb->max[0], aabb->max[1], aabb->max[2],
    // aabb->max[0], aabb->max[1], aabb->min[2],
    // aabb->max[0], aabb->min[1], aabb->min[2],
    // aabb->max[0], aabb->min[1], aabb->max[2],
    //
    // aabb->min[0], aabb->min[1], aabb->min[2],
    // aabb->min[0], aabb->min[1], aabb->max[2],
    // aabb->min[0], aabb->max[1], aabb->max[2],
    // aabb->min[0], aabb->max[1], aabb->min[2],
  };
  unsigned int indices[24] = {
    0, 1,
    1, 2,
    2, 3,
    3, 0,

    0, 6,
    1, 7,
    2, 4,
    3, 5,

    4, 5,
    5, 6,
    6, 7,
    7, 4
  };

  // Gen VAO, VBO, EBO
  glGenVertexArrays(1, &aabb->VAO);
  glGenBuffers(1, &aabb->VBO);
  glGenBuffers(1, &aabb->EBO);
  glBindVertexArray(aabb->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, aabb->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, aabb->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Configure attribute pointer, unbind
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Assign ints to the AABB struct

  // Unbind
  glBindVertexArray(0);
}

// For now, assume the user has created the shader program and set its uniforms
// Performance will be very bad because I'm generating new buffers
// to buffer data that might not change on every frame.
// Could make a solution with static buffer IDs and glBufferSubData later.
void AABB_render(struct AABB *aabb, mat4 model, mat4 view, mat4 projection){
  // Use shader, set uniforms
  shader_use(aabbShader);
  shader_set_mat4(aabbShader, "model", model);
  shader_set_mat4(aabbShader, "view", view);
  shader_set_mat4(aabbShader, "projection", projection);

  // Draw lines
  glLineWidth(2.0f);
  glBindVertexArray(aabb->VAO);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);

  // Cleanup
  // glBindVertexArray(0);
  // glDeleteVertexArrays(1, &VAO);
  // glDeleteBuffers(1, &VBO);
  // glDeleteBuffers(1, &EBO);
}
