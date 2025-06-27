#include "physics/debug_renderer.h"
#include "shader.h"
#include <math.h>
#include <signal.h>

// Shaders for wireframes (bounding volumes) and translucent objects (planes)
static Shader *wireframeShader;
static Shader *translucentShader;

void physics_debug_render(struct PhysicsWorld *physics_world, struct RenderContext *context){
  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->VBO);

    mat4 model;

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        // mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_AABB_render(box, context, model);
        break;
      case COLLIDER_SPHERE:
        struct Sphere *sphere = &body->collider.data.sphere;

        // mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, body->position);
        // Spheres are invariant to rotation
        // glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        // glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        // glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_sphere_render(sphere, context, model);
        break;
      case COLLIDER_PLANE:
        // debug_plane_render(body);
        break;
      default:
        break;
    }
  }
  for (unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body = &physics_world->dynamic_bodies[i];

    mat4 model;

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        struct AABB rotated_AABB = {0};
        mat4 eulerA;
        mat3 rotationA;

        vec3 rotation_rad;
        glm_vec3_copy(body->rotation, rotation_rad);
        glm_vec3_scale(rotation_rad, M_PI / 180.0f, rotation_rad); // Degrees to radians
        glm_euler_xyz(rotation_rad, eulerA);
        glm_mat4_pick3(eulerA, rotationA);
        vec3 translationA, scaleA;
        // glm_vec3_sub(box->center, body->position, translationA);
        glm_vec3_copy(body->position, translationA); // Use body position
        glm_vec3_copy(body->scale, scaleA); // Use body scale
        AABB_update(box, rotationA, translationA, scaleA, &rotated_AABB);

        glm_mat4_identity(model);
        // glm_translate(model, body->position);
        // glm_rotate_x(model, glm_rad(body->rotation[1]), model);
        // glm_rotate_y(model, glm_rad(body->rotation[0]), model);
        // glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        // glm_scale(model, body->scale);

        glBindVertexArray(body->VAO);
        glBindBuffer(GL_ARRAY_BUFFER, body->VBO);
        physics_debug_AABB_render(&rotated_AABB, context, model);
        break;
      case COLLIDER_SPHERE:
        struct Sphere *sphere = &body->collider.data.sphere;

        // mat4 model;
        glm_mat4_identity(model);
        vec3 translation;
        glm_vec3_add(body->position, sphere->center, translation);
        glm_translate(model, translation);
        // glm_translate(model, body->position);
        // Spheres are invariant to rotation
        // glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        // glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        // glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        glBindVertexArray(body->VAO);
        physics_debug_sphere_render(sphere, context, model);
        break;
      case COLLIDER_PLANE:
        // debug_plane_render(body);
        break;
      default:
        break;
    }
  }
}

void physics_debug_renderer_init(struct PhysicsWorld *physics_world){
  // wireframeShader = shader_create("shaders/physics/wireframe.vs", "shaders/physics/wireframe.fs");
  wireframeShader = shader_create("shaders/physics/common.vs", "shaders/physics/aabb.fs");
  translucentShader = shader_create("shaders/physics/common.vs", "shaders/physics/translucent.fs");

  if (!wireframeShader){
    fprintf(stderr, "Error: failed to create wireframe shader in debug_renderer_init\n");
    return;
  }
  if (!translucentShader){
    fprintf(stderr, "Error: failed to create wireframe shader in debug_renderer_init\n");
    return;
  }

  // Uniform block binding
  unsigned int uniform_block_index_wireframe = glGetUniformBlockIndex(wireframeShader->ID, "Matrices");
  unsigned int uniform_block_index_translucent = glGetUniformBlockIndex(translucentShader->ID, "Matrices");
  glUniformBlockBinding(wireframeShader->ID, uniform_block_index_wireframe, 0);
  glUniformBlockBinding(translucentShader->ID, uniform_block_index_translucent, 0);

  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        physics_debug_sphere_init(body);
        break;
      case COLLIDER_PLANE:
        // debug_plane_init(body);
        break;
      default:
        break;
    }
  }
  for (unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body = &physics_world->dynamic_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        physics_debug_sphere_init(body);
        break;
      case COLLIDER_PLANE:
        // debug_plane_init(body);
        break;
      default:
        break;
    }
  }
}

void physics_debug_AABB_init(struct PhysicsBody *body){
  if (!wireframeShader){
    fprintf(stderr, "Error: wireframe shader program not initialized\n");
    return;
  }

  struct AABB *aabb = &body->collider.data.aabb;

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

  // printf("INIT AABB DEBUG VERTICES\n");
  // for(int i = 0; i < 8; i++){
  //   printf("Vertex (%f %f %f)\n", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
  // }

  // Buffers
  glGenVertexArrays(1, &body->VAO);
  glGenBuffers(1, &body->VBO);
  glGenBuffers(1, &body->EBO);
  glBindVertexArray(body->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, body->VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

  // Configure attribute pointer, unbind
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Unbind VAO
  glBindVertexArray(0);
}

void physics_debug_sphere_init(struct PhysicsBody *body){
  if (!wireframeShader){
    fprintf(stderr, "Error: wireframe shader program not initialized\n");
    return;
  }

  struct Sphere *sphere = &body->collider.data.sphere;

  // Create sphere vertices and indices using stacks and sectors
  // (https://songho.ca/opengl/gl_sphere.html)
  int sector_count = 20;
  int stack_count = 20;
  float sector_step = (2 * M_PI) / 20;
  float stack_step = M_PI / 20;
  float sector_angle, stack_angle;

  int num_vertices = (stack_count + 1) * (sector_count + 1);
  int num_indices = (sector_count * ((2 * stack_count) - 2)) * 3;
  // int num_indices = (sector_count * (stack_count - 2) * 6) + (sector_count * 6);

  // Vertices
  float *vertices = (float *)malloc(num_vertices * 3 * sizeof(float));
  if (!vertices){
    fprintf(stderr, "Error: failed to allocate sphere vertices in physics_debug_sphere_init\n");
    return;
  }

  int vertex_index = 0;
  for (int i = 0; i <= stack_count; i++){
    stack_angle = (M_PI / 2) - (i * stack_step);
    float stack_xy = sphere->radius * cosf(stack_angle); // r * cos(phi)
    float z = sphere->radius * sinf(stack_angle);

    for (int j = 0; j <= sector_count; j++){
      sector_angle = j * sector_step;
      float x = stack_xy * cosf(sector_angle); // r * cos(phi) * cos(theta)
      float y = stack_xy * sinf(sector_angle); // r * cos(phi) * sin(theta)

      // vertices[vertex_index * 3] = sphere->center[0] + x;
      // vertices[vertex_index * 3 + 1] = sphere->center[1] + y;
      // vertices[vertex_index * 3 + 2] = sphere->center[2] + z;
      vertices[vertex_index * 3] = x;
      vertices[vertex_index * 3 + 1] = y;
      vertices[vertex_index * 3 + 2] = z;
      vertex_index++;
    }
  }

  // Indices
  unsigned int *indices = (unsigned int *)malloc(num_indices * sizeof(float));
  if (!indices){
    fprintf(stderr, "Error: failed to allocate sphere indices in physics_debug_sphere_init\n");
    return;
  }
  int k1, k2;
  unsigned int indices_index = 0;
  for (int i = 0; i < stack_count; i++){
    k1 = i * (sector_count + 1);
    k2 = k1 + sector_count + 1;

    for (int j = 0; j < sector_count; j++, k1++, k2++){
      // 2 triangles/sector (except first and last)
      if (i != 0){
        indices[indices_index++] = k1;
        indices[indices_index++] = k2;
        indices[indices_index++] = k1 + 1;
      }
      if (i != (stack_count - 1)){
        indices[indices_index++] = k1 + 1;
        indices[indices_index++] = k2;
        indices[indices_index++] = k2 + 1;
      }
    }
  }

  // Buffers
  glGenVertexArrays(1, &body->VAO);
  glGenBuffers(1, &body->VBO);
  glGenBuffers(1, &body->EBO);
  glBindVertexArray(body->VAO);

  glBindBuffer(GL_ARRAY_BUFFER, body->VBO);
  glBufferData(GL_ARRAY_BUFFER, num_vertices * 3 * sizeof(float), vertices, GL_DYNAMIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Configure attribute pointer, unbind
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Unbind VAO
  glBindVertexArray(0);
}

void physics_debug_plane_init(struct PhysicsBody *body){

}

void physics_debug_AABB_render(struct AABB *aabb, struct RenderContext *context, mat4 model){
  // Shader and uniforms
  shader_use(wireframeShader);
  shader_set_mat4(wireframeShader, "model", model);
  // shader_set_mat4(wireframeShader, "view", context->view_ptr);
  // shader_set_mat4(wireframeShader, "projection", context->projection_ptr);

  // Buffer new vertex data
  float vertices[24] = {
    aabb->center[0] + aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] + aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] + aabb->extents[2],

    aabb->center[0] - aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] - aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] - aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] + aabb->extents[2],
    aabb->center[0] - aabb->extents[0], aabb->center[1] + aabb->extents[1], aabb->center[2] - aabb->extents[2],
  };
  // for(int i = 0; i < 8; i++){
  //   printf("Vertex (%f %f %f)\n", vertices[i*3], vertices[i*3+1], vertices[i*3+2]);
  // }
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), &vertices);

  // Draw lines
  glLineWidth(2.0f);
  glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

  glBindVertexArray(0);
}

void physics_debug_sphere_render(struct Sphere *sphere, struct RenderContext *context, mat4 model){
  // Shader and uniforms
  shader_use(translucentShader);
  shader_set_mat4(translucentShader, "model", model);
  // shader_set_mat4(translucentShader, "view", context->view_ptr);
  // shader_set_mat4(translucentShader, "projection", context->projection_ptr);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, 2280, GL_UNSIGNED_INT, (void*)0);

  glBindVertexArray(0);
}

void physics_debug_plane_render(struct Plane *plane, struct RenderContext *context, mat4 model){
  // Disable back face culling for planes
}
