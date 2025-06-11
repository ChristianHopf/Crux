//#include <GLFW/glfw3.h>
#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <glad/glad.h>
#include <cglm/cglm.h>
#include <cglm/mat3.h>
#include "scene.h"
#include "player.h"
#include "skybox.h"
#include "text.h"
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
  scene->light = (Light *)malloc(sizeof(Light));
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

  // Entities
  scene->num_entities = 0;
  scene->max_entities = 5;
  scene->entities = (Entity *)malloc(scene->max_entities * sizeof(Entity));
  if (!scene->entities){
    printf("Error: failed to allocate scene entities\n");
    free(scene);
    return NULL;
  }

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

  struct Model *oiiaiModel = (struct Model *)malloc(sizeof(struct Model));
  if (!oiiaiModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(oiiaiModel, "resources/objects/oiiai/scene.gltf");
  Entity oiiai = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .velocity = {0.0f, -0.1f, 0.0f},
    .model = oiiaiModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = oiiai;

  struct Model *planeModel = (struct Model *)malloc(sizeof(struct Model));
  if (!planeModel){
    printf("Error: failed to allocate oiiaiModel\n");
    return NULL;
  }
  model_load(planeModel, "resources/basic/grass_plane/grass_plane.gltf");
  Entity plane = {
    .ID = 1,
    .position = {0.0f, -1.0f, 0.0f},
    .rotation = {0.0f, 0.0f, 0.0f},
    .scale = {3.0f, 3.0f, 3.0f},
    .velocity = {0.0f, 0.0f, 0.0f},
    .model = planeModel,
    .shader = shader
  };
  scene->entities[scene->num_entities++] = plane;

  // Create a PhysicsWorld and populate it with PhysicsBodies

  // Skybox
  scene->skybox = skybox_create();
  if (!scene->skybox){
    printf("Error: failed to create skybox in scene_create\n");
  }

  return scene;
}

void scene_update(Scene *scene, float deltaTime){
  // Skip update if the scene is paused
  if (scene->paused){
    return;
  }

  // Timing
  static float total_time = 0.0f;
  total_time += deltaTime;

  float rotationSpeed = 100.0f;
  float lightSpeed = 1.0f;

  // Update player
  player_update(&scene->player, deltaTime);

  // Perform primitive collision detection:
  // a single broad-phase check of every possible pair
  for(int i = 0; i < scene->num_entities-1; i++){
    Entity *entity = &scene->entities[i];

    // **********
    // MOVE TO PHYSICS_STEP
    //
    // Sequential method: move this entity by its velocity
    vec3 update;
    glm_vec3_copy(entity->velocity, update);
    glm_vec3_scale(update, deltaTime, update);
    glm_vec3_add(entity->position, update, entity->position);

    // Get matrix and vector to update current AABB into world space
    mat4 eulerA;
    mat3 aabbUpdateMatA;
    glm_euler_xyz(entity->rotation, eulerA);
    glm_mat4_pick3(eulerA, aabbUpdateMatA);
      
    vec3 aabbUpdateVecA;
    glm_vec3_copy(scene->entities[i].position, aabbUpdateVecA);
      
    struct AABB worldAABB_A = {0};
    AABB_update(&entity->model->aabb, aabbUpdateMatA, aabbUpdateVecA, &worldAABB_A);

    // Check for collision with every other entity
    for(int j = i+1; j < scene->num_entities; j++){
      // Get matrix and vector to update AABB A
      mat4 eulerB;
      mat3 aabbUpdateMatB;
      glm_euler_xyz(scene->entities[j].rotation, eulerB);
      glm_mat4_pick3(eulerB, aabbUpdateMatB);
      
      vec3 aabbUpdateVecB;
      glm_vec3_copy(scene->entities[j].position, aabbUpdateVecB);
      
      struct AABB worldAABB_B = {0};
      AABB_update(&scene->entities[j].model->aabb, aabbUpdateMatB, aabbUpdateVecB, &worldAABB_B);

      // Perform collision check
      if (AABB_intersect(&worldAABB_A, &worldAABB_B)){
        // printf("1\n");
        printf("Collision detected between the following aabbs:\n");
        print_aabb(&worldAABB_A);
        print_aabb(&worldAABB_B);
      }
      else{
        // printf("2\n");
        // printf("No collision detected between the following aabbs:\n");
        // print_aabb(&worldAABB_A);
        // print_aabb(&worldAABB_B);
      }
    }

    // Update translation vector
    // entity->position[1] = (float)sin(glfwGetTime()*4) / 4;
    // Update rotation vector
    // entity->rotation[1] -= rotationSpeed * deltaTime;
  }
  //
  // MOVE TO PHYSICS_STEP
  // **********

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

  // For each entity in the scene
  for(int i = 0; i < scene->num_entities; i++){
    // Bind its shader
    Entity *entity = &scene->entities[i];
    shader_use(entity->shader);

    // Get its model matrix
    mat4 model;
    glm_mat4_identity(model);
    // Translate
    glm_translate(model, entity->position);
    // Rotate
    glm_rotate_y(model, glm_rad(entity->rotation[1]), model);
    glm_rotate_x(model, glm_rad(entity->rotation[0]), model);
    glm_rotate_z(model, glm_rad(entity->rotation[2]), model);
    // Scale
    glm_scale(model, entity->scale);

    // Set normal matrix uniform
    mat3 transposed_mat3;
    mat3 normal;
    glm_mat4_pick3t(model, transposed_mat3);
    glm_mat3_inv(transposed_mat3, normal);
    shader_set_mat3(entity->shader, "normal", normal);

    // Set its model, view, and projection matrix uniforms
    shader_set_mat4(entity->shader, "model", model);
    shader_set_mat4(entity->shader, "view", view);
    shader_set_mat4(entity->shader, "projection", projection);

    // Lighting uniforms
    shader_set_vec3(entity->shader, "dirLight.direction", scene->light->direction);
    shader_set_vec3(entity->shader, "dirLight.ambient", scene->light->ambient);
    shader_set_vec3(entity->shader, "dirLight.diffuse", scene->light->diffuse);
    shader_set_vec3(entity->shader, "dirLight.specular", scene->light->specular);

    // Set camera position as viewPos in the fragment shader
    shader_set_vec3(entity->shader, "viewPos", scene->player.camera->position);

    // Draw model
    model_draw(entity->model, entity->shader);

    // Render the model's AABB
    if (scene->physics_view_mode){
      AABB_render(&entity->model->aabb, model, view, projection);
    }
  }

  // Skybox
  glDepthFunc(GL_LEQUAL);
  shader_use(scene->skybox->shader);

  // Set view and projection matrix uniforms
  mat3 view_skybox_mat3;
  mat4 view_skybox;
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
