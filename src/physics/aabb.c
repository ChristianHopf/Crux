#include "physics/aabb.h"
#include "physics/utils.h"
#include <cglm/vec3.h>

// Shader program for rendering AABB wireframes
static Shader *aabbShader;


// Merge b into a
void AABB_merge(struct AABB *a, struct AABB *b){
  a->min[0] = fminf(a->min[0], b->min[0]);
  a->min[1] = fminf(a->min[1], b->min[1]);
  a->min[2] = fminf(a->min[2], b->min[2]);

  a->max[0] = fmaxf(a->max[0], b->max[0]);
  a->max[1] = fmaxf(a->max[1], b->max[1]);
  a->max[2] = fmaxf(a->max[2], b->max[2]);
}

void AABB_update(struct AABB *src, mat3 rotation, vec3 translation, struct AABB *dest){
  // printf("Time to update AABB src and store in dest. Current values of src and dest:\n");
  // print_aabb(src);
  // print_aabb(dest);
  // For all three axes:
  for (int i = 0; i < 3; i++){
    // Add translation
    dest->min[i] = dest->max[i] = translation[i];

    // Form the min and max extents of this axis by summing smaller and larger terms
    for (int j = 0; j < 3; j++){
      float e = rotation[i][j] * src->min[j];
      float f = rotation[i][j] * src->max[j];
      if (e < f){
        dest->min[i] += e;
        dest->max[i] += f;
      } else{
        dest->min[i] += f;
        dest->max[i] += e;
      }
    }
  }
}

//Figure out an optimal algorithm for this later.
// If you update the minimum x component, for example,
// you certainly don't have to update the maximum x component.
void AABB_update_by_vertex(struct AABB *aabb, vec3 vertex){
  // Minimum
  aabb->min[0] = fminf(aabb->min[0], vertex[0]);
  aabb->min[1] = fminf(aabb->min[1], vertex[1]);
  aabb->min[2] = fminf(aabb->min[2], vertex[2]);

  // Maximum
  aabb->max[0] = fmaxf(aabb->max[0], vertex[0]);
  aabb->max[1] = fmaxf(aabb->max[1], vertex[1]);
  aabb->max[2] = fmaxf(aabb->max[2], vertex[2]);
}

// COLLISION TESTS
//
// Intersection between AABBs
bool AABB_intersect(struct AABB *a, struct AABB *b){
  if (a->max[0] < b->min[0] || a->min[0] > b->max[0]) return false;
  if (a->max[2] < b->min[2] || a->min[2] > b->max[2]) return false;
  if (a->max[1] < b->min[1] || a->min[1] > b->max[1]) return false;
  return true;
}

// Intersection between an AABB and a plane
bool AABB_intersect_plane(struct AABB *box, struct PlaneCollider *plane){
  // Get center and positive half-extents (switch AABB to c+h representation later)
  vec3 center;
  glm_vec3_add(box->max, box->min, center);
  glm_vec3_scale(center, 0.5f, center);
  vec3 extents;
  glm_vec3_sub(box->max, center, extents);

  // Get radius of the extents' projection interval onto the plane's normal
  float r =
    extents[0] * fabs(plane->normal[0]) +
    extents[1] * fabs(plane->normal[1]) +
    extents[2] * fabs(plane->normal[2]); 
  // Get distance from the center of AABB to the plane
  float s = glm_dot(plane->normal, center) - plane->distance;

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
    aabb->max[0], aabb->max[1], aabb->max[2],
    aabb->max[0], aabb->max[1], aabb->min[2],
    aabb->max[0], aabb->min[1], aabb->min[2],
    aabb->max[0], aabb->min[1], aabb->max[2],

    aabb->min[0], aabb->min[1], aabb->min[2],
    aabb->min[0], aabb->min[1], aabb->max[2],
    aabb->min[0], aabb->max[1], aabb->max[2],
    aabb->min[0], aabb->max[1], aabb->min[2],
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
