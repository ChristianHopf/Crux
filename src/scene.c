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

void scene_init(struct Scene *scene, char *scene_path){
  // Allocate Scene
  scene = (struct Scene *)malloc(sizeof(struct Scene));
  if (!scene){
    printf("Error: failed to allocate scene in scene_init\n");
    return;
  }

  // Set options
  scene->physics_view_mode = true;
  scene->paused = false;

  // Parse scene JSON
  unsigned char *scene_data = read_file(scene_path);

  const cJSON *shaders;
  const cJSON *models;
  const cJSON *meshes;
  const cJSON *lights;
  const cJSON *skybox;

  cJSON *scene_json = cJSON_Parse(scene_data);
  if (!scene_json){
    const char *error_ptr = cJSON_GetErrorPtr();
    if (!error_ptr){
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    cJSON_Delete(scene_JSON);
    return;
  }

  // Load and compile Shaders
  shaders = cJSON_GetObjectItemCaseSensitive(scene_json, "shaders");
  if (!cJSON_IsArray(shaders)){
    fprintf(stderr, "Error: failed to get shaders object in scene_init, shaders is either invalid or does not exist\n");
    return;
  }
  int num_shaders = cJSON_GetArraySize(shaders);
  Shader *shaders[num_shaders];

  int index = 0;
  cJSON_ArrayForeach(shader, shaders){
    cJSON *vertex = cJSON_GetObjectItemCaseSensitive(shader, "vertex");
    cJSON *fragment = cJSON_GetObjectItemCaseSensitive(shader, "fragment");

    if (!cJSON_IsString(vertex) || !cJSON_IsString(fragment)){
      fprintf(stderr, "Error: invalid JSON data in shaders[\"vertex\"]\n");
      cJSON_Delete(scene_json);
      return;
    }

    Shader *shader = shader_create("shaders/shader.vs", "shaders/dirlight/shader.fs");
    if (!shader){
      printf("Error: failed to create shader program\n");
    }
    shaders[index] = shader;
    index++;
  }

  // Load models
  models = cJSON_GetObjectItemCaseSensitive(scene_json, "models");
  if (!cJSON_IsArray(models)){
    fprintf(stderr, "Error: failed to get models array in scene_init, models is either invalid or does not exist\n");
    return;
  }
  int num_models = cJSON_GetArraySize(models);
  Model *models[num_models];

  index = 0;
  cJSON_ArrayForEach(model, models){
    if (!cJSON_IsString(model)){
      fprintf(stderr, "Error: invalid JSON data in models: model must be a string (filepath)\n");
      return;
    }

    struct Model *loaded_model = (struct Model *)malloc(sizeof(struct Model));
    if (!loaded_model){
      fprintf(stderr, "Error: failed to allocate model with path %s in scene_init\n", model);
      return;
    }
    if (!model_load(loaded_model, model)){
      fprintf(stderr, "Error: failed to load model with path %s in scene_init\n", model);
    }

    models[index] = loaded_model;
    index++;
  }

  // Create entities
  meshes = cJSON_GetObjectItemCaseSensitive(scene_json, "meshes");
  if (!meshes){
    fprintf(stderr, "Error: failed to get meshes object in scene_json, meshes is either invalid or does not exist\n");
    return;
  }
  cJSON *static_meshes = cJSON_GetObjectItemCaseSensitive(meshes, "static");
  cJSON *dynamic_meshes = cJSON_GetObjectItemCaseSensitive(meshes, "dynamic");
  if (!static_meshes){
    fprintf(stderr, "Error: failed to get meshes[\"static\"] object in scene_init, invalid or does not exist\n");
    return;
  }
  if (!dynamic_meshes){
    fprintf(stderr, "Error: failed to get meshes[\"dynamic\"] object in scene_init, invalid or does not exist\n");
    return;
  }

  // Process static meshes
  int num_static_meshes = cJSON_GetArraySize(static_meshes);
  struct Entity static_entities[num_static_meshes];

  index = 0;
  cJSON_ArrayForEach(mesh, static_meshes){
    // Create Entity
    struct Entity *entity = &static_entities[index];

    cJSON *model_index_json = cJSON_GetObjectItemCaseSensitive(mesh, "model_index");
    cJSON *shader_index_json = cJSON_GetObjectItemCaseSensitive(mesh, "shader_index");
    if(!cJSON_IsNumber(model_index_json)){
      fprintf(stderr, "Error: failed to get model_index in static mesh at index %d, either invalid or does not exist\n", index);
      return;
    }
    if(!cJSON_IsNumber(shader_index_json)){
      fprintf(stderr, "Error: failed to get shader_index in static mesh at index %d, either invalid or does not exist\n", index);
      return;
    }

    entity->model = models[model_index];
    entity->shader = shaders[shader_index];

    cJSON *position_json = cJSON_GetObjectItemCaseSensitive(mesh, "position");
    if (!cJSON_IsArray(position_json) || cJSON_GetArraySize(position) != 3){
      fprintf(stderr, "Error: failed to get position in static mesh at index %d, either invalid or does not exist\n", index);
      return;
    }
    entity->position[0] = cJSON_GetNumberValue(cJSON_GetArrayItem(position, 0));
    entity->position[1] = cJSON_GetNumberValue(cJSON_GetArrayItem(position, 1));
    entity->position[2] = cJSON_GetNumberValue(cJSON_GetArrayItem(position, 2));

    cJSON *rotation_json = cJSON_GetObjectItemCaseSensitive(mesh, "rotation");
    if (!cJSON_IsArray(rotation_json) || cJSON_GetArraySize(rotation) != 3){
      fprintf(stderr, "Error: failed to get rotation in static mesh at index %d, either invalid or does not exist\n", index);
      return;
    }
    entity->rotation[0] = cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 0));
    entity->rotation[1] = cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 1));
    entity->rotation[2] = cJSON_GetNumberValue(cJSON_GetArrayItem(rotation, 2));

    cJSON *scale_json = cJSON_GetObjectItemCaseSensitive(mesh, "scale");
    if (!cJSON_IsArray(scale_json) || cJSON_GetArraySize(scale) != 3){
      fprintf(stderr, "Error: failed to get scale in static mesh at index %d, either invalid or does not exist\n", index);
      return;
    }
    entity->scale[0] = cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 0));
    entity->scale[1] = cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 1));
    entity->scale[2] = cJSON_GetNumberValue(cJSON_GetArrayItem(scale, 2));

    // Process mesh collider
    cJSON *collider_json = cJSON_GetObjectItemCaseSensitive(mesh, "collider");
    if (!collider){
      fprintf(stderr, "Error: failed to get collider object in mesh at index %d, either invalid or does not exist\n", index);
      return;
    }

    

    cJSON *collider_type = cJSON_GetObjectItemCaseSensitive(collider_json, "type");
    if (!cJSON_IsNumber(collider_type)){
      fprintf(stderr, "Error: failed to get type in collider object in mesh at index %d, either invalid or does not exist\n", index);
      return;
    }

    cJSON *collider_data_json = cJSON_GetObjectItemCaseSensitive(collider_json, "data");
    if (!collider_data_json){
      fprintf(stderr, "Error: failed to get data in collider object in mesh at index %d, either invalid or does not exist\n", index);
      return;
    }

    switch(cJSON_GetNumberValue(collider_type)){
      // AABB
      case 0:
        // Get AABB, call physics_add_body with type and data.
        // Add a call to physics_create_world above
        break;
      // Plane
      case 1:
        break;
      default:
        break;
    }
  }

  // Process dynamic meshes
  int num_dynamic_meshes = cJSON_GetArraySize(dynamic_meshes);
  struct Entity *dynamic_entities[num_dynamic_meshes];

  index = 0;
  cJSON_ArrayForEach(mesh, dynamic_meshes){
    // Create Entity
  }
  
  // Lights
  lights = cJSON_GetObjectItemCaseSensitive(scene_json, "lights");
  if (!lights){
    fprintf(stderr, "Error: failed to get lights object in scene_json, lights is either invalid or does not exist\n");
    return;
  }

  int num_lights = cJSON_GetArraySize(lights);
  scene->light = (struct Light *)malloc(num_lights * sizeof(struct Light));
  if (!scene->light){
    fprintf(stderr, "Error: failed to allocate scene lights\n");
    return;
  }

  index = 0;
  cJSON_ArrayForEach(light, lights){
    // Create Light
  }
  
  // Skybox
  //
  // For now, only supports a cubemap skybox defined by 6 texture files.
  // The value in the JSON is the path to the directory that contains the files
  skybox = cJSON_GetObjectItemCaseSensitive(scene_json, "skybox");
  if (!cJSON_IsString(skybox)){
    fprintf(stderr, "Error: failed to get skybox from scene_json, skybox is either invalid or does not exist\n");
    return;
  }

  scene->skybox = skybox_create(skybox);

  return;
}

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
    .position = {0.0f, 2.0f, 0.0f},
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
  struct Entity planeEntity = {
    .ID = 1,
    .position = {0.0f, 0.0f, 0.0f},
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

  // Perform collision detection
  physics_step(scene->physics_world, delta_time);

  // Match entity position with updated PhysicsBody position
  for(int i = 0; i < scene->num_entities; i++){
    // printf("Time to sync %d entities with their physics bodies\n", scene->num_entities);
    glm_vec3_copy(scene->entities[i].physics_body->position, scene->entities[i].position);
    // print_glm_vec3(scene->entities[i].physics_body->position, "This entity's physics body position");
    // print_glm_vec3(scene->entities[i].position, "New entity position");
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
