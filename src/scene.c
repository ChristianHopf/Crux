//#include <GLFW/glfw3.h>
#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/mat3.h>
#include "scene.h"
#include "render_context.h"
#include "player.h"
#include "skybox.h"
#include "text.h"
#include "physics/world.h"
#include "physics/aabb.h"
#include "physics/utils.h"
#include "utils.h"

Scene *scene_create(bool physics_view_mode){
  // Allocate scene
  Scene *scene = (Scene *)malloc(sizeof(Scene));
  if (!scene){
    printf("Error: failed to allocate scene\n");
    return NULL;
  }

  // Physics view mode (render AABBs)
  scene->physics_view_mode = physics_view_mode;

  // Start unpaused
  scene->paused = false;

  // Light
  scene->light = (struct Light *)malloc(sizeof(struct Light));
  if (!scene->light){
    printf("Error: failed to allocate scene light\n");
    return NULL;
  }
  glm_vec3_copy((vec3){-0.2f, 0.0f, -0.3f}, scene->light->direction);
  glm_vec3_copy((vec3){0.2f, 0.2f, 0.2f}, scene->light->ambient);
  glm_vec3_copy((vec3){0.8f, 0.8f, 0.8f}, scene->light->diffuse);
  glm_vec3_copy((vec3){1.0f, 1.0f, 1.0f}, scene->light->specular);

  // Player
  player_init(&scene->player);

  // Shaders and entities for physics dev
  Shader *shader = shader_create("shaders/shader.vs", "shaders/dirlight/shader.fs");
  if (!shader){
    printf("Error: failed to create shader program\n");
    glfwTerminate();
    return NULL;
  }

  // Entities
  scene->num_entities = 0;
  scene->max_entities = 5;
  scene->entities = (struct Entity *)malloc(scene->max_entities * sizeof(struct Entity));
  if (!scene->entities){
    printf("Error: failed to allocate scene entities\n");
    free(scene);
    return NULL;
  }

  struct Model *oiiaiModel = (struct Model *)malloc(sizeof(struct Model));
  if (!oiiaiModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(oiiaiModel, "resources/objects/oiiai/scene.gltf");
  struct Entity oiiaiEntity = {
    .ID = 2,
    .position = {0.0f, 3.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .velocity = {0.0f, 0.0f, 0.0f},
    .physics_body = NULL,
    .model = oiiaiModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = oiiaiEntity;

  struct Model *planeModel = (struct Model *)malloc(sizeof(struct Model));
  if (!planeModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(planeModel, "resources/basic/grass_plane/grass_plane.gltf");
  // struct Level level = {
  //   .position = {0.0f, -1.0f, 0.0f},
  //   .rotation = {0.0f, 0.0f, 0.0f},
  //   .scale = {3.0f, 3.0f, 3.0f},
  //   .model = planeModel,
  //   .shader = shader
  // };
  // scene->level = level
  struct Entity planeEntity = {
    .ID = 1,
    .position = {0.0f, -1.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .velocity = {0.0f, 0.0f, 0.0f},
    .physics_body = NULL,
    .model = planeModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = planeEntity;

  // Create a PhysicsWorld and populate it with Colliders and PhysicsBodies
  scene->physics_world = physics_world_create();
  // Maybe make a plane_collider_create function to pass a normal and distance
  // scene->physics_world->level_plane = (struct PlaneCollider *)calloc(1, sizeof(struct PlaneCollider *));
  // glm_vec3_copy((vec3){0.0f, 1.0f, 0.0f}, scene->physics_world->level_plane->normal);
  // scene->physics_world->level_plane->distance = -1.0f;

  // Later, do one for static and one for dynamic entities
  for(int i = 0; i < scene->num_entities; i++){
    scene->entities[i].physics_body = physics_add_body(scene->physics_world, &scene->entities[i]);
  }

  // Skybox
  scene->skybox = skybox_create();
  if (!scene->skybox){
    printf("Error: failed to create skybox in scene_create\n");
  }

  return scene;
}

void scene_update(Scene *scene, float delta_time){
  // Skip update if the scene is paused
  if (scene->paused){
    return;
  }

  // Timing
  static float total_time = 0.0f;
  total_time += delta_time;

  float rotationSpeed = 100.0f;
  float lightSpeed = 1.0f;

  // Update player
  player_update(&scene->player, delta_time);

  // Sequential method: move entity, then check collisions
  // for(int i = 0; i < scene->num_entities; i++){
  //   struct Entity *entity = &scene->entities[i];
  //   // Update velocity according to gravity
  //   float gravity = 1.0f * delta_time;
  //   entity->velocity[1] -= gravity;
  //
  //   glm_vec3_muladds(entity->velocity, delta_time, entity->position);
  // }

  // Perform collision detection
  physics_step(scene->physics_world, delta_time);

  // Match entity position with updated PhysicsBody position
  for(int i = 0; i < scene->num_entities; i++){
    printf("Time to sync %d entities with their physics bodies\n", scene->num_entities);
    glm_vec3_copy(scene->entities[i].physics_body->position, scene->entities[i].position);
    // print_glm_vec3(scene->entities[i].physics_body->position, "This entity's physics body position");
    print_glm_vec3(scene->entities[i].position, "New entity position");
  }

  // Update translation vector
  // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
  // Update rotation vector
  // entity->rotation[1] -= rotationSpeed * deltaTime;

  // Update light
  scene->light->direction[0] = (float)sin(lightSpeed * total_time);
  //scene->light->direction[1] += y;
  scene->light->direction[2] = (float)cos(lightSpeed * total_time);
}

void scene_render(Scene *scene){
  // Render (clear color and depth buffer bits)
  glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Get view and projection matrices
  mat4 view;
  mat4 projection;
  camera_get_view_matrix(scene->player.camera, view);
  glm_perspective(glm_rad(scene->player.camera->fov), 800.0f / 600.0f, 0.1f, 100.0f, projection);

  // Create a RenderContext, which is simply
  // a collection of parameters for rendering the Level and Entities
  struct RenderContext context = {
    .light_ptr = scene->light,
    .view_ptr = view,
    .projection_ptr = projection,
    .camera_position_ptr = scene->player.camera->position,
    .physics_view_mode = scene->physics_view_mode
  };

  // Draw level
  // level_render(&scene->level, &context);

  // Draw entities
  for(int i = 0; i < scene->num_entities; i++){
    entity_render(&scene->entities[i], &context);
  }

  // Draw skybox
  skybox_render(scene->skybox, &context);

  // Render text
  text_render("Crux Engine 0.1", 4.0f, 744.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
}

void scene_pause(Scene *scene){
  bool prev = scene->paused;
  scene->paused = !prev;
}

void free_scene(Scene *scene){
  if (scene){
    // Rewrite this to actually free everything
    free(scene->entities);
    free(scene->player.camera);
    free(scene->light);
    free(scene->skybox->shader);
    free(scene->skybox);
    free(scene);
  }
}
