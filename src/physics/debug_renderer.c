#include "physics/debug_renderer.h"
#include "shader.h"

// Shaders for wireframes (bounding volumes) and translucent objects (planes)
static Shader *wireframeShader;
static Shader *translucentShader;

void physics_debug_render(struct PhysicsWorld *physics_world, struct RenderContext *context){
  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->VBO);

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_AABB_render(box, context, model);
        break;
      case COLLIDER_SPHERE:
        // debug_sphere_render(body);
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

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, body->VBO);

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        mat4 model;
        glm_mat4_identity(model);
        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_AABB_render(box, context, model);
        break;
      case COLLIDER_SPHERE:
        // debug_sphere_render(body);
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
  wireframeShader = shader_create("shaders/physics/aabb.vs", "shaders/physics/aabb.fs");
  translucentShader = shader_create("shaders/physics/translucent.vs", "shaders/physics/translucent.fs");

  if (!wireframeShader){
    fprintf(stderr, "Error: failed to create wireframe shader in debug_renderer_init\n");
    return;
  }
  if (!translucentShader){
    fprintf(stderr, "Error: failed to create wireframe shader in debug_renderer_init\n");
    return;
  }

  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        // debug_sphere_init(body);
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
        // debug_sphere_init(body);
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
  // Create the AABB shader
  if (!wireframeShader){
    fprintf(stderr, "Error: wireframe shader program not initialized\n");
    return;
  }

  struct AABB *aabb = &body->collider.data.aabb;
  print_aabb(aabb);

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

  // Gen VAO, VBO, EBO
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

}

void physics_debug_plane_init(struct PhysicsBody *body){

}

void physics_debug_AABB_render(struct AABB *aabb, struct RenderContext *context, mat4 model){
  // Shader and uniforms
  shader_use(wireframeShader);
  shader_set_mat4(wireframeShader, "model", model);
  shader_set_mat4(wireframeShader, "view", context->view_ptr);
  shader_set_mat4(wireframeShader, "projection", context->projection_ptr);

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
  // Maybe make spheres use the translucent shader too?
}

void physics_debug_plane_render(struct Plane *plane, struct RenderContext *context, mat4 model){
  // Disable back face culling for planes
}
