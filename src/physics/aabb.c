#include "physics/aabb.h"

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

  a->max[0] = fmaxf(a->max[0], b->max[0]);
  a->max[1] = fmaxf(a->max[1], b->max[1]);
  a->max[2] = fmaxf(a->max[2], b->max[2]);
}

// Figure out an optimal algorithm for this later.
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
    printf("Successfully created AABB shader and assigned its poiner to static aabbShader\n");
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
  printf("Assigned VAO, VBO, and EBO to AABB: %d %d %d\n", aabb->VAO, aabb->VBO, aabb->EBO);

  // Unbind
  glBindVertexArray(0);
}

// For now, assume the user has created the shader program and set its uniforms
// Performance will be very bad because I'm generating new buffers
// to buffer data that might not change on every frame.
// Could make a solution with static buffer IDs and glBufferSubData later.
void AABB_render(struct AABB *aabb, mat4 model, mat4 view, mat4 projection){
  printf("AABB {\n");
  printf("  min: (%.2f, %.2f, %.2f)\n", aabb->min[0], aabb->min[1], aabb->min[2]);
  printf("  max: (%.2f, %.2f, %.2f)\n", aabb->max[0], aabb->max[1], aabb->max[2]);
  printf("}\n");
  // Define vertices and indices for the box, based on min and max
  // float vertices[24] = {
  //   aabb->max[0], aabb->max[1], aabb->max[2],
  //   aabb->max[0], aabb->max[1], aabb->min[2],
  //   aabb->max[0], aabb->min[1], aabb->min[2],
  //   aabb->max[0], aabb->min[1], aabb->max[2],
  //
  //   aabb->min[0], aabb->min[1], aabb->min[2],
  //   aabb->min[0], aabb->min[1], aabb->max[2],
  //   aabb->min[0], aabb->max[1], aabb->max[2],
  //   aabb->min[0], aabb->max[1], aabb->min[2],
  // };
  // unsigned int indices[24] = {
  //   0, 1,
  //   1, 2,
  //   2, 3,
  //   3, 0,
  //
  //   0, 6,
  //   1, 7,
  //   2, 4,
  //   3, 5,
  //
  //   4, 5,
  //   5, 6,
  //   6, 7,
  //   7, 4
  // };
  //
  // // Gen VAO, VBO, EBO
  // GLuint VAO, VBO, EBO;
  // glGenVertexArrays(1, &VAO);
  // glGenBuffers(1, &VBO);
  // glGenBuffers(1, &EBO);
  // glBindVertexArray(VAO);
  //
  // // Buffer vertices and indices (might want to use GL_DYNAMIC_DRAW for rendering them as they move later?)
  // glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), vertices, GL_STATIC_DRAW);
  //
  // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  // glBufferData(GL_ELEMENT_ARRAY_BUFFER, 24 * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Config attrib pointer
  // glEnableVertexAttribArray(0);
  // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Draw lines
  shader_use(aabbShader);
  printf("using aabbshader with id: %d\n", aabbShader->ID);
  shader_set_mat4(aabbShader, "model", model);
  shader_set_mat4(aabbShader, "view", view);
  shader_set_mat4(aabbShader, "projection", projection);

  glLineWidth(2.0f);
  printf("Time to draw AABB lines\n");
  glBindVertexArray(aabb->VAO);
  printf("Successfully bound AABB VAO\n");
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);

  // Cleanup
  // glBindVertexArray(0);
  // glDeleteVertexArrays(1, &VAO);
  // glDeleteBuffers(1, &VBO);
  // glDeleteBuffers(1, &EBO);
}
