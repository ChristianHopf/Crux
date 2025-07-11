#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <cglm/cglm.h>
#include <cglm/mat3.h>
#include "scene.h"
#include "render_context.h"
#include "player.h"
#include "skybox.h"
#include "text.h"
#include "physics/world.h"
#include "physics/aabb.h"
#include "physics/debug_renderer.h"
#include "physics/utils.h"
#include "audio_manager.h"
#include "utils.h"

struct Scene *scene_init(char *scene_path){
  // Allocate Scene
  struct Scene *scene = (struct Scene *)calloc(1, sizeof(struct Scene));
  if (!scene){
    printf("Error: failed to allocate scene in scene_init\n");
    return NULL;
  }

  // Set options
  scene->physics_debug_mode = true;

  // Parse scene JSON
  const char *scene_data = (const char *)read_file(scene_path);

  const cJSON *shaders_json;
  const cJSON *models_json;
  const cJSON *sounds_json;
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
    index++;
  }

  // Link shader uniform blocks to binding points
  for (int i = 0; i < num_shaders; i++){
    unsigned int uniform_block_index = glGetUniformBlockIndex(shaders[i]->ID, "Matrices");
    if (uniform_block_index != GL_INVALID_INDEX){
      glUniformBlockBinding(shaders[i]->ID, uniform_block_index, 0);
    }
  }
  // Generate uniform buffer objects
  unsigned int uboMatrices;
  glGenBuffers(1, &uboMatrices);
  glBindBuffer(GL_UNIFORM_BUFFER, uboMatrices);
  glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(mat4), NULL, GL_STATIC_DRAW);
  
  glBindBufferRange(GL_UNIFORM_BUFFER, 0, uboMatrices, 0, 2 * sizeof(mat4));
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  scene->ubo_matrices = uboMatrices;

  // Load models
  models_json = cJSON_GetObjectItemCaseSensitive(scene_json, "models");
  if (!cJSON_IsArray(models_json)){
    fprintf(stderr, "Error: failed to get models array in scene_init, models is either invalid or does not exist\n");
    return NULL;
  }
  int num_models = cJSON_GetArraySize(models_json);
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
    index++;
  }

  // Load music
  sounds_json = cJSON_GetObjectItemCaseSensitive(scene_json, "sounds");
  if (!sounds_json){
    fprintf(stderr, "Error: failed to get sounds object in scene_init, sounds is either invalid or does not exist\n");
    return NULL;
  }
  cJSON *music_json = cJSON_GetObjectItemCaseSensitive(sounds_json, "music");
  if (!music_json){
    fprintf(stderr, "Error: failed to get music in sounds object in scene_init, music is either inavlid or does not exist\n");
    return NULL;
  }
  // Only one music stream for now, could later start multiple streams for other ambient loops
  char *music_path = cJSON_GetStringValue(music_json);
  // audio_stream_create(music_path);

  // Load sound effects
  cJSON *sound_effects_json = cJSON_GetObjectItemCaseSensitive(sounds_json, "effects");
  if (!cJSON_IsArray(sound_effects_json)){
    fprintf(stderr, "Error: failed to get effects array in sounds object in scene_init, effects is either invalid or does not exist\n");
    return NULL;
  }
  // num_sound_effects = cJSON_GetArraySize(sound_effects_json);
  const cJSON *effect_json = NULL;
  cJSON_ArrayForEach(effect_json, sound_effects_json){
    cJSON *path;
    cJSON *name;
    audio_sound_effect_create("resources/sfx/vineboom.wav", "vine_boom");
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
  scene->static_entities = (struct Entity *)calloc(num_static_meshes, sizeof(struct Entity));
  scene->num_static_entities = num_static_meshes;

  scene_process_meshes_json(static_meshes, models, shaders, scene->static_entities, scene->physics_world, false);

  // Process dynamic meshes
  int num_dynamic_meshes = cJSON_GetArraySize(dynamic_meshes);
  scene->dynamic_entities = (struct Entity *)calloc(num_dynamic_meshes, sizeof(struct Entity));
  scene->num_dynamic_entities = num_dynamic_meshes;

  scene_process_meshes_json(dynamic_meshes, models, shaders, scene->dynamic_entities, scene->physics_world, true);

  scene->max_entities = 64;

  // Init physics debug renderer
  if (scene->physics_debug_mode){
    physics_debug_renderer_init(scene->physics_world);
  }
  
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
    scene_process_light_json(light_json, light);
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
  // Timing
  static float total_time = 0.0f;
  total_time += delta_time;

  // Update player
  player_update(&scene->player, delta_time);

  // Perform collision detection
  physics_step(scene->physics_world, delta_time);

  // Match entity audio source and PhysicsBody positions with entity position
  for(int i = 0; i < scene->num_dynamic_entities; i++){
    struct Entity *entity = &scene->dynamic_entities[i];
    glm_vec3_copy(entity->physics_body->position, scene->dynamic_entities[i].position);
    glm_vec3_copy(entity->physics_body->rotation, scene->dynamic_entities[i].rotation);
    alSource3f(entity->audio_component->source_id, AL_POSITION, entity->position[0], entity->position[1], entity->position[2]);
    ALenum position_error = alGetError();
    if (position_error != AL_NO_ERROR){
      fprintf(stderr, "Error matching Entity audio_source position with entity position in scene_update: %d\n", position_error);
    }
  }

  // Update light
  float lightSpeed = 1.0f;
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
  glm_perspective(glm_rad(scene->player.camera->fov), 1920.0f / 1080.0f, 0.1f, 100.0f, projection);

  // Create a RenderContext, which is simply
  // a collection of parameters for rendering the Level and Entities
  struct RenderContext context = {
    .light_ptr = scene->lights,
    .view_ptr = &view,
    .projection_ptr = &projection,
    .camera_position_ptr = &scene->player.camera->position,
  };

  // Set view and projection matrices in matrices UBO
  glBindBuffer(GL_UNIFORM_BUFFER, scene->ubo_matrices);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), view);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), projection);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Draw entities
  for(int i = 0; i < scene->num_static_entities; i++){
    entity_render(&scene->static_entities[i], &context);
  }
  float oiiai_start_time = glfwGetTime();
  for(int i = 0; i < scene->num_dynamic_entities; i++){
    entity_render(&scene->dynamic_entities[i], &context);
  }
  // printf("OIIAI RENDER TIME: %.2f ms\n", (glfwGetTime() - oiiai_start_time) * 1000.0);

  // Draw skybox
  skybox_render(scene->skybox, &context);

  // Draw physics debug volumes
  if (scene->physics_debug_mode){
    physics_debug_render(scene->physics_world, &context);
  }

  // Render text
  text_render("Crux Engine 0.2", 4.0f, 1058.0f, 1.0f, (vec3){1.0f, 1.0f, 1.0f});
}

// void scene_pause(struct Scene *scene){
//   scene->paused = true;
//
//   // Pause audio
//   audio_pause();
// }
//
// void scene_unpause(struct Scene *scene){
//   scene->paused = false;
//
//   // Unpause audio
//   audio_unpause();
// }

//     // Rewrite this to actually free everything
//     free(scene->entities);
//     free(scene->player.camera);
//     free(scene->light);
//     free(scene->skybox->shader);
//     free(scene->skybox);
//     free(scene);
//   }
// }

void scene_process_meshes_json(cJSON *meshes, struct Model **models, Shader **shaders, struct Entity *entities, struct PhysicsWorld *physics_world, bool dynamic){
  int index = 0;
  cJSON *mesh_json;
  cJSON_ArrayForEach(mesh_json, meshes){
    // Create Entity
    struct Entity *entity = &entities[index];

    cJSON *model_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "model_index");
    cJSON *shader_index_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "shader_index");
    if(!cJSON_IsNumber(model_index_json)){
      fprintf(stderr, "Error: failed to get model_index in mesh at index %d, either invalid or does not exist\n", index);
      return;
    }
    if(!cJSON_IsNumber(shader_index_json)){
      fprintf(stderr, "Error: failed to get shader_index in mesh at index %d, either invalid or does not exist\n", index);
      return;
    }

    entity->model = models[(int)cJSON_GetNumberValue(model_index_json)];
    entity->shader = shaders[(int)cJSON_GetNumberValue(shader_index_json)];

    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "position"), entity->position);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "rotation"), entity->rotation);
    scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "scale"), entity->scale);
    if (dynamic){
      scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(mesh_json, "velocity"), entity->velocity);
    }

    // Process mesh collider
    cJSON *collider_json = cJSON_GetObjectItemCaseSensitive(mesh_json, "collider");
    if (!collider_json){
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
          return;
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
          return;
        }

        plane.distance = cJSON_GetNumberValue(distance);

        collider.type = type;
        collider.data.plane = plane;
        break;
      default:
        break;
    }
    entity->physics_body = physics_add_body(physics_world, entity, collider, dynamic);
    index++;

    // AudioComponent
    entity->audio_component = audio_component_create(entity, 0);
  }
}

void scene_process_light_json(cJSON *light_json, struct Light *light){
  // Get data (direction, ambient, diffuse, specular)
  cJSON *light_data_json = cJSON_GetObjectItemCaseSensitive(light_json, "data");
  if (!light_data_json){
    fprintf(stderr, "Error: failed to get data object in light at index %d, either invalid or does not exist\n", index);
    return;
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
