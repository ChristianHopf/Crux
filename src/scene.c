//#include <GLFW/glfw3.h>
#include <cglm/euler.h>
#include <cglm/mat4.h>
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
  // Shader *planeShader = shader_create("shaders/basic/plane.vs", "shaders/dirlight/shader.fs");
  // if (!planeShader){
  //   printf("Error: failed to create plane shader program\n");
  //   glfwTerminate();
  //   return NULL;
  // }
  
  // Level
  struct Model *planeModel = (struct Model *)malloc(sizeof(struct Model));
  if (!planeModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(planeModel, "resources/basic/grass_plane/grass_plane.gltf");
  struct Level level = {
    .position = {0.0f, -1.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .model = planeModel,
    .shader = shader
  };
  scene->level = level;

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
  struct Entity oiiai = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .velocity = {0.0f, -0.2f, 0.0f},
    .model = oiiaiModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = oiiai;

  // struct Model *planeModel = (struct Model *)malloc(sizeof(struct Model));
  // if (!planeModel){
  //   printf("Error: failed to allocate oiiaiModel\n");
  //   return NULL;
  // }
  // model_load(planeModel, "resources/basic/grass_plane/grass_plane.gltf");
  // struct Entity plane = {
  //   .ID = 1,
  //   .position = {0.0f, -1.0f, 0.0f},
  //   .rotation = {0.0f, 0.0f, 0.0f},
  //   .scale = {3.0f, 3.0f, 3.0f},
  //   .velocity = {0.0f, 0.0f, 0.0f},
  //   .model = planeModel,
  //   .shader = shader
  // };
  // scene->entities[scene->num_entities++] = plane;

  //Create a PhysicsWorld and populate it with PhysicsBodies
  scene->physics_world = physics_world_create();
  for(int i = 0; i < scene->num_entities; i++){
    physics_add_body(scene->physics_world, &scene->entities[i]);
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
  for(int i = 0; i < scene->num_entities; i++){
    struct Entity *entity = &scene->entities[i];
    vec3 update;
    glm_vec3_copy(entity->velocity, update);
    glm_vec3_scale(update, delta_time, update);
    glm_vec3_add(entity->position, update, entity->position);
  }

  // Perform collision detection
  physics_step(scene->physics_world, delta_time);

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
    .light = scene->light,
  };
  glm_mat4_copy(view, context.view);
  glm_mat4_copy(projection, context.projection);
  glm_vec3_copy(scene->player.camera->position, context.camera_position);
  context.physics_view_mode = scene->physics_view_mode;

  // Draw level
  level_render(&scene->level, &context);

  // Draw entities
  for(int i = 0; i < scene->num_entities; i++){
    entity_render(&scene->entities[i], &context);
  }

  // Skybox
  glDepthFunc(GL_LEQUAL);
  shader_use(scene->skybox->shader);

  // Build skybox view matrix, ignoring translation
  mat3 view_skybox_mat3;
  mat4 view_skybox;
  glm_mat4_identity(view_skybox);
  glm_mat4_pick3(view, view_skybox_mat3);
  glm_mat4_ins3(view_skybox_mat3, view_skybox);

  shader_set_mat4(scene->skybox->shader, "view", view_skybox);
  shader_set_mat4(scene->skybox->shader, "projection", projection);

  // Bind vertex array
  glBindVertexArray(scene->skybox->cubemapVAO);
  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, scene->skybox->cubemap_texture_id);
  // Draw triangles
  glDrawArrays(GL_TRIANGLES, 0, 36);

  glBindVertexArray(0);
  glDepthFunc(GL_LESS);

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
