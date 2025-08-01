#include "physics/debug_renderer.h"
#include "shader.h"
#include <math.h>
#include <signal.h>

// Shaders for wireframes (bounding volumes) and translucent objects (planes)
static Shader *wireframeShader;
static Shader *translucentShader;

void physics_debug_render(struct PhysicsWorld *physics_world, struct RenderContext *context){
  // Render player bodies
  // TODO refactor the switch into its own function
  for (unsigned int i = 0; i < physics_world->num_player_bodies; i++){
    struct PhysicsBody *body = &physics_world->player_bodies[i];

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->EBO);

    mat4 model;
    glm_mat4_identity(model);

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_AABB_render(box, context, model);
        break;
      case COLLIDER_SPHERE:
        struct Sphere *sphere = &body->collider.data.sphere;

        glm_translate(model, body->position);
        // Spheres are invariant to rotation
        // glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        // glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        // glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_sphere_render(sphere, context, model);
        break;
      case COLLIDER_CAPSULE:
        struct Capsule *capsule = &body->collider.data.capsule;

        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_capsule_render(capsule, context, model);
        break;
      case COLLIDER_PLANE:
        // debug_plane_render(body);
        break;
      default:
        break;
    }
  }

  // Render static bodies
  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->EBO);

    mat4 model;
    glm_mat4_identity(model);

    switch(body->collider.type){
      case COLLIDER_AABB:
        // Get updated AABB and model matrix
        struct AABB *box = &body->collider.data.aabb;

        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_AABB_render(box, context, model);
        break;
      case COLLIDER_SPHERE:
        struct Sphere *sphere = &body->collider.data.sphere;

        glm_translate(model, body->position);
        // Spheres are invariant to rotation
        // glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        // glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        // glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_sphere_render(sphere, context, model);
        break;
      case COLLIDER_CAPSULE:
        struct Capsule *capsule = &body->collider.data.capsule;

        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_capsule_render(capsule, context, model);
        break;
      case COLLIDER_PLANE:
        // debug_plane_render(body);
        break;
      default:
        break;
    }
  }

  // Render dynamic bodies
  for (unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body = &physics_world->dynamic_bodies[i];

    glBindVertexArray(body->VAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, body->EBO);

    mat4 model;
    glm_mat4_identity(model);

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
      case COLLIDER_CAPSULE:
        struct Capsule *capsule = &body->collider.data.capsule;

        glm_translate(model, body->position);
        glm_rotate_y(model, glm_rad(body->rotation[1]), model);
        glm_rotate_x(model, glm_rad(body->rotation[0]), model);
        glm_rotate_z(model, glm_rad(body->rotation[2]), model);
        glm_scale(model, body->scale);

        physics_debug_capsule_render(capsule, context, body->scene_node->local_transform);
        // physics_debug_capsule_render(capsule, context, model);
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

  // Init players' bodies
  for (unsigned int i = 0; i < physics_world->num_player_bodies; i++){
    struct PhysicsBody *body = &physics_world->player_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        physics_debug_sphere_init(body);
        break;
      case COLLIDER_CAPSULE:
        physics_debug_capsule_init(body);
        break;
      case COLLIDER_PLANE:
        // debug_plane_init(body);
        break;
      default:
        break;
    }
  }

  // Init static bodies
  for (unsigned int i = 0; i < physics_world->num_static_bodies; i++){
    struct PhysicsBody *body = &physics_world->static_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        physics_debug_sphere_init(body);
        break;
      case COLLIDER_CAPSULE:
        physics_debug_capsule_init(body);
        break;
      case COLLIDER_PLANE:
        // debug_plane_init(body);
        break;
      default:
        break;
    }
  }

  // Init dynamic bodies
  for (unsigned int i = 0; i < physics_world->num_dynamic_bodies; i++){
    struct PhysicsBody *body = &physics_world->dynamic_bodies[i];
    switch(body->collider.type){
      case COLLIDER_AABB:
        physics_debug_AABB_init(body);
        break;
      case COLLIDER_SPHERE:
        physics_debug_sphere_init(body);
        break;
      case COLLIDER_CAPSULE:
        physics_debug_capsule_init(body);
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

  free(vertices);
  free(indices);
}

void physics_debug_capsule_init(struct PhysicsBody *body){
  if (!wireframeShader){
    fprintf(stderr, "Error: wireframe shader program not initialized\n");
    return;
  }

  // Transform capsule to world space
  struct Capsule *capsule = &body->collider.data.capsule;
  struct Capsule world_capsule = {0};
  // TODO give Player a SceneNode
  // if (body->scene_node){
  //   print_glm_mat4(body->scene_node->world_transform, "capsule init world transform");
  //   glm_mat4_mulv3(body->scene_node->world_transform, capsule->segment_A, 1.0f, world_capsule.segment_A);
  //   glm_mat4_mulv3(body->scene_node->world_transform, capsule->segment_B, 1.0f, world_capsule.segment_B);
  //   vec3 world_scale;
  //   glm_decompose_scalev(body->scene_node->parent_node->world_transform, world_scale);
  //   world_capsule.radius  = capsule->radius * world_scale[0];
  // }
  // else{
    glm_vec3_copy(capsule->segment_A, world_capsule.segment_A);
    glm_vec3_copy(capsule->segment_B, world_capsule.segment_B);
    world_capsule.radius = capsule->radius;
  // }

  // Get axis and length of cylinder
  vec3 axis;
  glm_vec3_sub(world_capsule.segment_B, world_capsule.segment_A, axis);
  // glm_vec3_sub(capsule->segment_B, capsule->segment_A, axis);
  float length = glm_vec3_norm(axis);
  glm_vec3_normalize(axis);

  // Stacks and sectors
  int sector_count = 20;
  int stack_count = 20;
  // stack_count stacks per hemisphere, one extra stack for the cylinder
  int total_stacks = stack_count * 2 + 1;
  float sector_step = (2 * M_PI) / sector_count;
  float stack_step = M_PI / stack_count;

  // Vertices and indices for cylinder
  int num_vertices = (total_stacks + 1) * (sector_count + 1);
  // printf("Num vertices is %d\n", num_vertices);
  int num_indices = total_stacks * sector_count * 6;
  // int num_indices = (sector_count * (total_stacks - 1)) * 6;
  // printf("A capsule has %d indices\n", num_indices);

  // Allocate vertices
  float *vertices = (float *)malloc(num_vertices * 3 * sizeof(float));
  if (!vertices) {
    fprintf(stderr, "Error: failed to allocate capsule vertices in physics_debug_capsule_init\n");
    return;
  }

  // Get axis and rotation matrix for rotating generated capsule to the capsule's axis
  vec3 z_axis = {0.0f, 0.0f, 1.0f};
  vec3 rotation_axis;
  glm_vec3_cross(z_axis, axis, rotation_axis);
  float rotation_angle = acosf(glm_vec3_dot(z_axis, axis));

  mat4 rotation_matrix;
  // If the magnitude of the rotation axis is positive, the axes are not parallel
  if (glm_vec3_norm(rotation_axis) > 0.0f) {
    glm_vec3_normalize(rotation_axis);
    glm_rotate_make(rotation_matrix, rotation_angle, rotation_axis);
  } else {
    // If the magnitude of the rotation axis is 0, and the dot product of the axes is not negative,
    // the axes are aligned, and no rotation is needed.
    // If the magnitude of the rotation axis is 0, and the dot product of the axes is negative,
    // the axes are opposite (z and -z), rotate 180 degrees around the x axis.
    glm_mat4_identity(rotation_matrix);
    if (glm_vec3_dot(z_axis, axis) < 0) {
      glm_rotate_make(rotation_matrix, M_PI, (vec3){1.0f, 0.0f, 0.0f});
    }
  }

  // Generate vertices
  int vertex_index = 0;
  // For each stack
  for (int i = 0; i <= total_stacks; i++){
    float t;
    vec3 center;
    float stack_radius;
    float z;
    
    // Bottom hemisphere:
    // - t is from -1 to 0
    // - center is segment_A
    // - 
    if (i < stack_count){
      t = ((float)i / stack_count);
      // glm_vec3_scale(axis, -capsule->radius, center);
      glm_vec3_copy(world_capsule.segment_A, center);
      stack_radius = world_capsule.radius * cosf(t * M_PI / 2.0f);
      z = -world_capsule.radius * sinf(t * M_PI / 2.0f);
    }
    else if (i == stack_count){
      glm_vec3_copy(world_capsule.segment_A, center);
      stack_radius = world_capsule.radius;
      z = 0.0f;
    }
    // Cylinder:
    // - t is from 0 to 1
    // - center of sector is interpolated between segment_A and segment_B
    else if (i == stack_count + 1){
      glm_vec3_copy(world_capsule.segment_B, center);
      stack_radius = world_capsule.radius;
      z = 0.0f;
    }
    // Top hemisphere:
    // - t is from 1 to 2
    // - center is segment_B
    else {
      t = ((float)(i - (stack_count + 1)) / stack_count);
      glm_vec3_copy(world_capsule.segment_B, center);
      stack_radius = world_capsule.radius * cosf(t * M_PI / 2.0f);
      z = world_capsule.radius * sinf(t * M_PI / 2.0f);
    }

    // For each sector, get x and y coordinates to make (sector_count + 1) vertices per stack
    for (int j = 0; j <= sector_count; j++){
      float sector_angle = j * sector_step;
      float x = stack_radius * cosf(sector_angle);
      float y = stack_radius * sinf(sector_angle);

      // Rotate and translate vertex to capsule axis and center
      vec3 vertex, rotated_vertex;
      vertex[0] = x;
      vertex[1] = y;
      vertex[2] = z;
      glm_mat4_mulv3(rotation_matrix, vertex, 1.0f, rotated_vertex);
      glm_vec3_add(rotated_vertex, center, rotated_vertex);

      vertices[vertex_index * 3] = rotated_vertex[0];
      vertices[vertex_index * 3 + 1] = rotated_vertex[1];
      vertices[vertex_index * 3 + 2] = rotated_vertex[2];
      vertex_index++;
    }
  }
  // printf("Successfully generated %d capsule vertices\n", vertex_index);

  // Allocate indices
  unsigned int *indices = (unsigned int*)malloc(num_indices * sizeof(unsigned int));
  if (!indices){
    fprintf(stderr, "Error: failed to allocate indices in physics_debug_capsule_init\n");
    free(vertices);
    return;
  }

  // Generate indices for two triangles per quad
  unsigned int indices_index = 0;
  for (int i = 0; i < total_stacks; i++){
    int k1 = i * (sector_count + 1);
    int k2 = k1 + sector_count + 1;

    for (int j = 0; j < sector_count; j++, k1++, k2++){
      indices[indices_index++] = k1;
      indices[indices_index++] = k1 + 1;
      indices[indices_index++] = k2;

      indices[indices_index++] = k1 + 1;
      indices[indices_index++] = k2 + 1;
      indices[indices_index++] = k2;
    }
  }
  // printf("Successfully generated %d capsule indices\n", indices_index);

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

  free(vertices);
  free(indices);
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

void physics_debug_capsule_render(struct Capsule *capsule, struct RenderContext *context, mat4 model){
  // printf("Rendering capsule\n");
  // Shader and uniforms
  shader_use(translucentShader);
  shader_set_mat4(translucentShader, "model", model);
  // shader_set_mat4(translucentShader, "view", context->view_ptr);
  // shader_set_mat4(translucentShader, "projection", context->projection_ptr);

  // Draw triangles
  glDrawElements(GL_TRIANGLES, 4920, GL_UNSIGNED_INT, (void*)0);

  glBindVertexArray(0);
}

void physics_debug_plane_render(struct Plane *plane, struct RenderContext *context, mat4 model){
  // Disable back face culling for planes
}
