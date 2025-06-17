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

struct Scene *scene_init(char *scene_path){
  // Allocate Scene
  struct Scene *scene = (struct Scene *)calloc(1, sizeof(struct Scene));
  if (!scene){
    printf("Error: failed to allocate scene in scene_init\n");
    return NULL;
  }

  // Set options
  scene->physics_view_mode = true;
  scene->paused = false;

  // Parse scene JSON
  const char *scene_data = (const char *)read_file(scene_path);

  const cJSON *shaders_json;
  const cJSON *models_json;
  const cJSON *meshes;
  const cJSON *lights_json;
  const cJSON *skybox_json;

  cJSON *scene_json = cJSON_Parse(scene_data);
  if (!scene_json){
    const char *error_ptr = cJSON_GetErrorPtr();
    if (!error_ptr){
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    printf("Failed to parse json\n");
    cJSON_Delete(scene_json);
    return NULL;
  }

  // Load and compile Shaders
  shaders_json = cJSON_GetObjectItemCaseSensitive(scene_json, "shaders");
  if (!cJSON_IsArray(shaders_json)){
    fprintf(stderr, "Error: failed to get shaders object in scene_init, shaders is either invalid or does not exist\n");
    return NULL;
  }
  int num_shaders = cJSON_GetArraySize(shaders_json);
  Shader *shaders[num_shaders];

  int index = 0;
  const cJSON *shader = NULL;
  cJSON_ArrayForEach(shader, shaders_json){
    cJSON *vertex = cJSON_GetObjectItemCaseSensitive(shader, "vertex");
    cJSON *fragment = cJSON_GetObjectItemCaseSensitive(shader, "fragment");

    if (!cJSON_IsString(vertex) || !cJSON_IsString(fragment)){
      fprintf(stderr, "Error: invalid JSON data in shaders[\"vertex\"]\n");
      cJSON_Delete(scene_json);
      return NULL;
    }
    char *vertex_path = cJSON_GetStringValue(vertex);
    char *fragment_path = cJSON_GetStringValue(fragment);

    Shader *shader = shader_create(vertex_path, fragment_path);
    if (!shader){
      printf("Error: failed to create shader program\n");
    }
    shaders[index] = shader;
    printf("Loaded shader with id %d\n", shader->ID);
    index++;
  }

  // Load models
  models_json = cJSON_GetObjectItemCaseSensitive(scene_json, "models");
  if (!cJSON_IsArray(models_json)){
    fprintf(stderr, "Error: failed to get models array in scene_init, models is either invalid or does not exist\n");
    return NULL;
  }
  int num_models = cJSON_GetArraySize(models_json);
  printf("Number of models to load: %d\n", num_models);
  struct Model *models[num_models];

  index = 0;
  cJSON *model_json;
  cJSON_ArrayForEach(model_json, models_json){
    if (!cJSON_IsString(model_json)){
      fprintf(stderr, "Error: invalid JSON data in models: model must be a string (filepath)\n");
      return NULL;
    }
    char *model_path = cJSON_GetStringValue(model_json);

    struct Model *loaded_model = (struct Model *)malloc(sizeof(struct Model));
    if (!loaded_model){
      fprintf(stderr, "Error: failed to allocate model with path %s in scene_init\n", model_path);
      return NULL;
    }
    if (!model_load(loaded_model, model_path)){
      fprintf(stderr, "Error: failed to load model with path %s in scene_init\n", model_path);
    }

    models[index] = loaded_model;
    printf("Loaded model with filepath %s\n", model_path);
    index++;
  }

  // Create entities and populate PhysicsWorld
  meshes = cJSON_GetObjectItemCaseSensitive(scene_json, "meshes");
  if (!meshes){
    fprintf(stderr, "Error: failed to get meshes object in scene_json, meshes is either invalid or does not exist\n");
    return NULL;
  }
  cJSON *static_meshes = cJSON_GetObjectItemCaseSensitive(meshes, "static");
  cJSON *dynamic_meshes = cJSON_GetObjectItemCaseSensitive(meshes, "dynamic");
  if (!static_meshes){
    fprintf(stderr, "Error: failed to get meshes[\"static\"] object in scene_init, invalid or does not exist\n");
    return NULL;
  }
  if (!dynamic_meshes){
    fprintf(stderr, "Error: failed to get meshes[\"dynamic\"] object in scene_init, invalid or does not exist\n");
    return NULL;
  }

  scene->physics_world = physics_world_create();

  // Process static meshes
  int num_static_meshes = cJSON_GetArraySize(static_meshes);
  scene->static_entities = (struct Entity *)malloc(num_static_meshes * sizeof(struct Entity));
  scene->num_static_entities = num_static_meshes;
  
  index = 0;
  cJSON *mesh_json;
  cJSON_ArrayForEach(mesh_json, static_meshes){
    // Create Entity
    struct Entity *entity = &scene->static_entities[index];

    cJSON *model_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "model_index");
    cJSON *shader_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "shader_index");
    if(!cJSON_IsNumber(model_index_json)){
      fprintf(stderr, "Error: failed to get model_index in static mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    if(!cJSON_IsNumber(shader_index_json)){
      fprintf(stderr, "Error: failed to get shader_index in static mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    printf("Model index is %d\n", (int)cJSON_GetNumberValue(model_index_json));

    entity->model = models[(int)cJSON_GetNumberValue(model_index_json)];
    entity->shader = shaders[(int)cJSON_GetNumberValue(shader_index_json)];

    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "position"), entity->position);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "rotation"), entity->rotation);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "scale"), entity->scale);

    // Process mesh collider
    cJSON *collider_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "collider");
    if (!collider_json){
      fprintf(stderr, "Error: failed to get collider object in mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    cJSON *collider_type = cJSON_GetObjectItemCaseSensitive(collider_json, "type");
    if (!cJSON_IsNumber(collider_type)){
      fprintf(stderr, "Error: failed to get type in collider object in mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    cJSON *collider_data_json = cJSON_GetObjectItemCaseSensitive(collider_json, "data");
    if (!collider_data_json){
      fprintf(stderr, "Error: failed to get data in collider object in mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }

    ColliderType type = cJSON_GetNumberValue(collider_type);
    struct Collider collider;
    switch(type){
      case COLLIDER_AABB:
        struct AABB aabb;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "center"), aabb.center);
        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "extents"), aabb.extents);

        aabb.initialized = true;

        collider.type = type;
        collider.data.aabb = aabb;
        break;
      case COLLIDER_SPHERE:
        struct Sphere sphere;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "center"), sphere.center);

        cJSON *radius = cJSON_GetObjectItemCaseSensitive(collider_data_json, "radius");
        if(!cJSON_IsNumber(radius)){
          fprintf(stderr, "Error: failed to get radius in collider object in static mesh at index %d, either invalid or does not exist\n", index);
          return NULL;
        }
        sphere.radius = cJSON_GetNumberValue(radius);

        collider.type = type;
        collider.data.sphere = sphere;
        break;
      case COLLIDER_PLANE:
        struct Plane plane;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "normal"), plane.normal);

        cJSON *distance = cJSON_GetObjectItemCaseSensitive(collider_data_json, "distance");
        if(!cJSON_IsNumber(distance)){
          fprintf(stderr, "Error: failed to get distance in collider object in static mesh at index %d, either invalid or does not exist\n", index);
          return NULL;
        }

        plane.distance = cJSON_GetNumberValue(distance);

        collider.type = type;
        collider.data.plane = plane;
        break;
      default:
        break;
    }
    printf("Added Entity to scene->static_entities at index %d:\n", index);
    printf("Shader id: %d, model index: %d\n", (int)cJSON_GetNumberValue(shader_index_json), (int)cJSON_GetNumberValue(model_index_json));
    printf("Model directory: %s\n", entity->model->directory);
    print_glm_vec3(entity->position, "Entity position");
    print_glm_vec3(entity->rotation, "Entity rotation");
    print_glm_vec3(entity->scale, "Entity scale");
    print_glm_vec3(collider.data.plane.normal, "Plane normal");
    printf("Plane distance: %f\n", collider.data.plane.distance);

    entity->physics_body = physics_add_body(scene->physics_world, entity, collider, false);
    index++;
  }

  // Process dynamic meshes
  int num_dynamic_meshes = cJSON_GetArraySize(dynamic_meshes);
  scene->dynamic_entities = (struct Entity *)malloc(num_dynamic_meshes * sizeof(struct Entity));
  scene->num_dynamic_entities = num_dynamic_meshes;

  index = 0;
  cJSON_ArrayForEach(mesh_json, dynamic_meshes){
    // Create Entity
    struct Entity *entity = &scene->dynamic_entities[index];

    cJSON *model_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "model_index");
    cJSON *shader_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "shader_index");
    if(!cJSON_IsNumber(model_index_json)){
      fprintf(stderr, "Error: failed to get model_index in dynamic mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    if(!cJSON_IsNumber(shader_index_json)){
      fprintf(stderr, "Error: failed to get shader_index in dynamic mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }

    entity->model = models[(int)cJSON_GetNumberValue(model_index_json)];
    entity->shader = shaders[(int)cJSON_GetNumberValue(shader_index_json)];

    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "position"), entity->position);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "rotation"), entity->rotation);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "scale"), entity->scale);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "velocity"), entity->velocity);

    // Process mesh collider
    cJSON *collider_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "collider");
    if (!collider_json){
      fprintf(stderr, "Error: failed to get collider object in dynamic mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    cJSON *collider_type = cJSON_GetObjectItemCaseSensitive(collider_json, "type");
    if (!cJSON_IsNumber(collider_type)){
      fprintf(stderr, "Error: failed to get type in collider object in dynamic mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }
    cJSON *collider_data_json = cJSON_GetObjectItemCaseSensitive(collider_json, "data");
    if (!collider_data_json){
      fprintf(stderr, "Error: failed to get data in collider object in dynamic mesh at index %d, either invalid or does not exist\n", index);
      return NULL;
    }

    ColliderType type = cJSON_GetNumberValue(collider_type);
    struct Collider collider;
    switch(type){
      case COLLIDER_AABB:
        struct AABB aabb;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "center"), aabb.center);
        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "extents"), aabb.extents);

        aabb.initialized = true;

        collider.type = type;
        collider.data.aabb = aabb;
        break;
      case COLLIDER_SPHERE:
        struct Sphere sphere;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "center"), sphere.center);

        cJSON *radius = cJSON_GetObjectItemCaseSensitive(collider_data_json, "radius");
        if(!cJSON_IsNumber(radius)){
          fprintf(stderr, "Error: failed to get radius in collider object in static mesh at index %d, either invalid or does not exist\n", index);
          return NULL;
        }
        sphere.radius = cJSON_GetNumberValue(radius);

        collider.type = type;
        collider.data.sphere = sphere;
        break;
      case COLLIDER_PLANE:
        struct Plane plane;

        // vec3 normal;
        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "normal"), plane.normal);

        cJSON *distance = cJSON_GetObjectItemCaseSensitive(collider_data_json, "distance");
        if(!cJSON_IsNumber(distance)){
          fprintf(stderr, "Error: failed to get distance in collider object in static mesh at index %d, either invalid or does not exist\n", index);
          return NULL;
        }

        plane.distance = cJSON_GetNumberValue(distance);

        collider.type = type;
        collider.data.plane = plane;
        break;
      default:
        break;
    }
    entity->physics_body = physics_add_body(scene->physics_world, entity, collider, true);
    index++;
  }
  scene->max_entities = 64;
  
  // Lights
  lights_json = cJSON_GetObjectItemCaseSensitive(scene_json, "lights");
  if (!lights_json){
    fprintf(stderr, "Error: failed to get lights object in scene_json, lights is either invalid or does not exist\n");
    return NULL;
  }

  int num_lights = cJSON_GetArraySize(lights_json);
  scene->lights = (struct Light *)malloc(num_lights * sizeof(struct Light));
  if (!scene->lights){
    fprintf(stderr, "Error: failed to allocate scene lights\n");
    return NULL;
  }

  index = 0;
  cJSON *light_json;
  cJSON_ArrayForEach(light_json, lights_json){
    struct Light *light = &scene->lights[index];

    // Get data (direction, ambient, diffuse, specular)
    cJSON *light_data_json = cJSON_GetObjectItemCaseSensitive(light_json, "data");
    if (!light_data_json){
      fprintf(stderr, "Error: failed to get data object in light at index %d, either invalid or does not exist\n", index);
      return NULL;
    }

    vec3 direction, ambient, diffuse, specular;
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(light_data_json, "direction"), direction);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(light_data_json, "ambient"), ambient);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(light_data_json, "diffuse"), diffuse);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(light_data_json, "specular"), specular);

    glm_vec3_copy(direction, light->direction);
    glm_vec3_copy(ambient, light->ambient);
    glm_vec3_copy(diffuse, light->diffuse);
    glm_vec3_copy(specular, light->specular);

    index++;
  }

  // Player
  player_init(&scene->player);
  
  // Skybox
  //
  // For now, only supports a cubemap skybox defined by 6 texture files.
  // The value in the JSON is the path to the directory that contains the files
  skybox_json = cJSON_GetObjectItemCaseSensitive(scene_json, "skybox");
  if (!cJSON_IsString(skybox_json)){
    fprintf(stderr, "Error: failed to get skybox from scene_json, skybox is either invalid or does not exist\n");
    return NULL;
  }

  char *skybox_dir = cJSON_GetStringValue(skybox_json);
  scene->skybox = skybox_create(skybox_dir);

  return scene;
}

void scene_update(struct Scene *scene, float delta_time){
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
  for(int i = 0; i < scene->num_dynamic_entities; i++){
    struct Entity *entity = &scene->dynamic_entities[i];
    // printf("Time to sync %d entities with their physics bodies\n", scene->num_entities);
    glm_vec3_copy(entity->physics_body->position, scene->dynamic_entities[i].position);
  }

  // Update light
  scene->lights[0].direction[0] = (float)sin(lightSpeed * total_time);
  //scene->light->direction[1] += y;
  scene->lights[0].direction[2] = (float)cos(lightSpeed * total_time);
}

void scene_render(struct Scene *scene){
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
    .light_ptr = scene->lights,
    .view_ptr = &view,
    .projection_ptr = &projection,
    .camera_position_ptr = &scene->player.camera->position,
    .physics_view_mode = scene->physics_view_mode
  };

  // Draw level
  // level_render(&scene->level, &context);

  // Draw entities
  for(int i = 0; i < scene->num_static_entities; i++){
    entity_render(&scene->static_entities[i], &context);
  }
  for(int i = 0; i < scene->num_dynamic_entities; i++){
    entity_render(&scene->dynamic_entities[i], &context);
  }

  // Draw skybox
  skybox_render(scene->skybox, &context);

  // Render text
  text_render("Crux Engine 0.1", 4.0f, 744.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
}

void scene_pause(struct Scene *scene){
  bool prev = scene->paused;
  scene->paused = !prev;
}

// void free_scene(struct Scene *scene){
//   if (scene){
//     // Rewrite this to actually free everything
//     free(scene->entities);
//     free(scene->player.camera);
//     free(scene->light);
//     free(scene->skybox->shader);
//     free(scene->skybox);
//     free(scene);
//   }
// }

void scene_process_static_meshes(){

}

void scene_process_dynamic_meshes(){

}

void scene_process_vec3_json(cJSON *vec3_json, vec3 dest){
  if (!cJSON_IsArray(vec3_json) || cJSON_GetArraySize(vec3_json) != 3){
    fprintf(stderr, "Error: failed to get %s vector, either invalid or does not exist\n", cJSON_GetStringValue(vec3_json));
    return;
  }
  dest[0] = cJSON_GetNumberValue(cJSON_GetArrayItem(vec3_json, 0));
  dest[1] = cJSON_GetNumberValue(cJSON_GetArrayItem(vec3_json, 1));
  dest[2] = cJSON_GetNumberValue(cJSON_GetArrayItem(vec3_json, 2));
}
