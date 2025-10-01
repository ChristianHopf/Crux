#include <cglm/euler.h>
#include <cglm/mat4.h>
#include <cglm/vec3.h>
#include <cglm/cglm.h>
#include <cglm/mat3.h>
#include "entity.h"
#include "scene.h"
#include "audio_manager.h"
#include "render_context.h"
#include "model.h"
#include "player.h"
#include "skybox.h"
#include "camera.h"
#include "text.h"
#include "inventory.h"
#include "physics/world.h"
#include "physics/aabb.h"
#include "physics/debug_renderer.h"
#include "physics/utils.h"
#include "event.h"
#include "engine.h"
#include "utils.h"

// Don't need more than this right now, since the SceneManager has nothing
// but a pointer to a Scene.
bool scene_manager_init(struct SceneManager *scene_manager){
  scene_manager->active_scene = NULL;
  return true;
}

void scene_manager_destroy(struct SceneManager *scene_manager){
  if (!scene_manager) return;
  scene_manager_unload_scene(scene_manager);
}

void scene_manager_load_scene(struct SceneManager *scene_manager, const char *path){
  if (!scene_manager){
    fprintf(stderr, "Error: scene_manager is NULL in scene_manager_load_scene\n");
    return;
  }

  scene_manager_unload_scene(scene_manager);
  scene_manager->active_scene = scene_load(path);
  if (!scene_manager->active_scene){
    fprintf(stderr, "Error: failed to load scene %s\n in scene_manager_load_scene\n", path);
  }
}

void scene_manager_unload_scene(struct SceneManager *scene_manager){
  if (!scene_manager || !scene_manager->active_scene) return;

  scene_free(scene_manager->active_scene);
  scene_manager->active_scene = NULL;
}

struct Scene *scene_load(const char *scene_path){
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
  const cJSON *lights_json;
  const cJSON *skybox_json;

  cJSON *scene_json = cJSON_Parse(scene_data);
  if (!scene_json){
    const char *error_ptr = cJSON_GetErrorPtr();
    if (!error_ptr){
      fprintf(stderr, "Error before: %s\n", error_ptr);
    }
    fprintf(stderr, "Error: failed to parse json in scene_load\n");
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
  scene->shaders = (Shader **)calloc(num_shaders, sizeof(Shader *));
  if (!scene->shaders){
    fprintf(stderr, "Error: failed to allocate scene->shaders in scene_init\n");
    return NULL;
  }
  for (int i = 0; i < num_shaders; i++){
    scene->shaders[i] = NULL;
  }
  scene->num_shaders = num_shaders;

  int index = 0;
  const cJSON *shader_data = NULL;
  cJSON_ArrayForEach(shader_data, shaders_json){
    cJSON *vertex = cJSON_GetObjectItemCaseSensitive(shader_data, "vertex");
    cJSON *fragment = cJSON_GetObjectItemCaseSensitive(shader_data, "fragment");

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
    scene->shaders[index] = shader;
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
  scene->models = (struct Model **)calloc(num_models, sizeof(struct Model *));
  if (!scene->models){
    fprintf(stderr, "Error: failed to allocate scene->shaders in scene_init\n");
    return NULL;
  }
  for (int i = 0; i < num_models; i++){
    scene->models[i] = NULL;
  }
  scene->num_models = num_models;

  index = 0;
  cJSON *model_json;
  cJSON_ArrayForEach(model_json, models_json){
    if (!cJSON_IsString(model_json)){
      fprintf(stderr, "Error: invalid JSON data in models: model must be a string (filepath)\n");
      return NULL;
    }
    char *model_path = cJSON_GetStringValue(model_json);

    struct Model *loaded_model = (struct Model *)calloc(1, sizeof(struct Model));
    if (!loaded_model){
      fprintf(stderr, "Error: failed to allocate model with path %s in scene_init\n", model_path);
      return NULL;
    }
    if (!model_load(loaded_model, model_path)){
      fprintf(stderr, "Error: failed to load model with path %s in scene_init\n", model_path);
    }

    models[index] = loaded_model;
    scene->models[index] = loaded_model;
    index++;
  }

  // Load music
  struct AudioManager *audio_manager = engine_get_audio_manager();
  if (!audio_manager){
    fprintf(stderr, "Error: failed to get audio_manager in scene_load\n");
    return NULL;
  }
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
    audio_sound_effect_create(audio_manager, "resources/sfx/vineboom.wav", "vine_boom");
    audio_sound_effect_create(audio_manager, "resources/sfx/itempickup.mp3", "item_pickup");
  }

  scene->physics_world = physics_world_create();

  // Allocate array of entities
  cJSON *entity_count_json = cJSON_GetObjectItemCaseSensitive(scene_json, "entity_count");
  if (!cJSON_IsNumber(entity_count_json)){
    fprintf(stderr, "Error: failed to get entity_count in scene_init, either invalid or does not exist\n");
    return NULL;
  }
  int entity_count = cJSON_GetNumberValue(entity_count_json);
  scene->entities = (struct Entity **)calloc(entity_count, sizeof(struct Entity *));
  if (!scene->entities){
    fprintf(stderr, "Error: failed to allocate scene->entities in scene_init\n");
    return NULL;
  }
  scene->num_entities = 0;

  // Process nodes for scene graph
  cJSON *nodes_json = cJSON_GetObjectItemCaseSensitive(scene_json, "nodes");
  if (!nodes_json){
    fprintf(stderr, "Error: failed to get nodes object in scene_init, invalid or does not exist\n");
    return NULL;
  }
  scene->root_node = (struct SceneNode *)calloc(1, sizeof(struct SceneNode));
  if (!scene->root_node){
    fprintf(stderr, "Error: failed to allocate root SceneNode in scene_init\n");
    return NULL;
  }

  // Build scene graph and fill entities array
  // scene_process_node_json(scene, nodes_json, scene->root_node, NULL, models, shaders, scene->physics_world);
  // scene->max_entities = 64;

  // Allocate Components
  scene->max_render_components = 32;
  scene->render_components = (struct RenderComponent *)calloc(scene->max_render_components, sizeof(struct RenderComponent));
  if (!scene->render_components){
    fprintf(stderr, "Error: failed to allocate scene RenderComponents in scene_init\n");
    return NULL;
  }
  scene->num_render_components = 0;

  // - AudioComponents
  scene->max_audio_components = 32;
  scene->audio_components = (struct AudioComponent *)calloc(scene->max_audio_components, sizeof(struct AudioComponent));
  if (!scene->audio_components){
    fprintf(stderr, "Error: failed to allocate scene AudioComponents in scene_init\n");
    return NULL;
  }
  scene->num_audio_components = 0;

  // - CameraComponents
  scene->max_camera_components = 8;
  scene->camera_components = (struct CameraComponent *)calloc(scene->max_camera_components, sizeof(struct CameraComponent));
  if (!scene->camera_components){
    fprintf(stderr, "Error: failed to allocate scene CameraComponents in scene_init\n");
    return NULL;
  }
  scene->num_camera_components = 0;

  // - PlayerComponents
  scene->max_player_components = 8;
  scene->player_components = (struct PlayerComponent *)calloc(scene->max_player_components, sizeof(struct PlayerComponent));
  if (!scene->player_components){
    fprintf(stderr, "Error: failed to allocate scene PlayerComponents in scene_init\n");
    return NULL;
  }
  scene->num_player_components = 0;

  // InventoryComponents
  scene->max_inventory_components = scene->max_player_components;
  scene->inventory_components = (struct InventoryComponent *)calloc(scene->max_inventory_components, sizeof(struct InventoryComponent));
  if (!scene->inventory_components){
    fprintf(stderr, "Error: failed to allocate InventoryComponents in scene_init\n");
    return NULL;
  }
  scene->num_inventory_components = 0;

  scene_process_node_json(scene, nodes_json, scene->root_node, NULL, models, shaders, scene->physics_world);
  scene->max_entities = 64;

  // Create player
  scene_player_create(scene, models[2], shaders[0],
                                (vec3){0.0f, 0.0f, 2.0f},
                                (vec3){0.0f, 180.0f, 0.0f},
                                (vec3){1.0f, 1.0f, 1.0f},
                                (vec3){0.0f, 0.0f, 0.0f},
                                (vec3){0.0f, 0.0f, 0.0f},
                                1.75f, false, 5, true);

  struct Collider player_collider = {
    .type = 2,
    .data.capsule = {
      .segment_A = {0.0f, 0.25f, 0.0f},
      .segment_B = {0.0f, 1.75f, 0.0f},
      .radius = 0.25f
    }
  };

  // Add Player PhysicsBodies
  for (unsigned int i = 0; i < scene->num_player_components; i++){
    struct PlayerComponent *player_component = &scene->player_components[i];
    struct Entity *entity = scene_get_entity_by_entity_id(scene, player_component->entity_id);
    // TODO figure out a better way to get a node by entity id when I don't need
    // any other information.
    struct SceneNode *scene_node;
    int child_index, final_child_index;
    scene_get_node_by_entity_id(scene->root_node, entity->id, &child_index, &final_child_index, &scene_node);
    if (scene_node){
      entity->physics_body = physics_add_player(scene->physics_world, scene_node, entity, player_collider);
    }
  }

  // Player inventory
  for (unsigned int i = 0; i < scene->num_player_components; i++){
    struct InventoryComponent *inventory_component = &scene->inventory_components[i];
    memcpy(inventory_component->entity_id, scene->player_components[i].entity_id, 16);
    inventory_component->capacity = 5;
    inventory_component->items = (struct ItemComponent *)calloc(inventory_component->capacity, sizeof(struct ItemComponent));
    inventory_component->size = 0;
    scene->num_inventory_components++;
  }

  // Lights
  lights_json = cJSON_GetObjectItemCaseSensitive(scene_json, "lights");
  if (!lights_json){
    fprintf(stderr, "Error: failed to get lights object in scene_json, lights is either invalid or does not exist\n");
    return NULL;
  }

  int num_lights = cJSON_GetArraySize(lights_json);
  scene->lights = (struct Light *)calloc(num_lights, sizeof(struct Light));
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

  // ItemRegistry
  const cJSON *items_json = cJSON_GetObjectItemCaseSensitive(scene_json, "items");
  if (!cJSON_IsArray(items_json)){
    fprintf(stderr, "Error: failed to get items object from scene_json, either invalid or does not exist\n");
    return NULL;
  }
  scene_process_items_json(scene, items_json);

  // Init physics debug renderer
  if (scene->physics_debug_mode){
    physics_debug_renderer_init(scene->physics_world);
  }
  
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

  // Perform collision detection
  physics_step(scene->physics_world, delta_time);

  // Process event queue
  game_event_queue_process();

  // Update player
  player_update(scene, scene->local_player_entity_id, delta_time);
  scene_node_update(scene, scene->root_node);
  // struct PlayerComponent *player = scene->player_components[0];
  // inventory_print(&scene->item_registry, scene_get_inventory_by_entity_id(scene, player->entity_id));
  // printf("Successfully printed inventory\n");

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
  struct CameraComponent *camera = scene_get_camera_by_entity_id(scene, scene->local_player_entity_id);
  camera_get_view_matrix(camera, view);
  glm_perspective(glm_rad(camera->fov), 1920.0f / 1080.0f, 0.1f, 100.0f, projection);

  // Create a RenderContext, which is simply
  // a collection of parameters for rendering the Level and Entities
  struct RenderContext context = {
    .light_ptr = scene->lights,
    .view_ptr = &view,
    .projection_ptr = &projection,
    .camera_position_ptr = &camera->position,
  };

  // Set view and projection matrices in matrices UBO
  glBindBuffer(GL_UNIFORM_BUFFER, scene->ubo_matrices);
  glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(mat4), view);
  glBufferSubData(GL_UNIFORM_BUFFER, sizeof(mat4), sizeof(mat4), projection);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);

  // Sort meshes by opaque, mask, transparent, additive
  struct RenderItem *opaque_items = NULL;
  struct RenderItem *mask_items = NULL;
  struct RenderItem *transparent_items = NULL;
  struct RenderItem *additive_items = NULL;
  unsigned int num_opaque_items = 0;
  unsigned int num_mask_items = 0;
  unsigned int num_transparent_items = 0;
  unsigned int num_additive_items = 0;

  // Allocate RenderItem arrays
  unsigned int num_render_items = 0;
  // scene_get_render_item_count(scene->root_node, &num_render_items);
  for (unsigned int i = 0; i < scene->num_render_components; i++){
    num_render_items += scene->render_components[i].model->num_meshes;
  }
  opaque_items = (struct RenderItem *)calloc(num_render_items, sizeof(struct RenderItem));
  if (!opaque_items){
    fprintf(stderr, "Error: failed to allocate opaque RenderItems in scene_render\n");
    // When rendering, make sure to check whether opaque_items is valid before trying to render them
  }
  mask_items = (struct RenderItem *)calloc(num_render_items, sizeof(struct RenderItem));
  if (!mask_items){
    fprintf(stderr, "Error: failed to allocate mask RenderItems in scene_render\n");
  }
  transparent_items = (struct RenderItem *)calloc(num_render_items, sizeof(struct RenderItem));
  if (!transparent_items){
    fprintf(stderr, "Error: failed to allocate transparent RenderItems in scene_render\n");
  }
  additive_items = (struct RenderItem *)calloc(num_render_items, sizeof(struct RenderItem));
  if (!additive_items){
    fprintf(stderr, "Error: failed to allocate additive RenderItems in scene_render\n");
  }

  // TODO will eventually just look at some kind of array of "RenderComponents"
  scene_get_render_items(scene, camera->position, &opaque_items, &num_opaque_items, &mask_items, &num_mask_items, &transparent_items, &num_transparent_items, &additive_items, &num_additive_items);
  // scene_get_render_items(scene->root_node, camera->position, &opaque_items, &num_opaque_items, &mask_items, &num_mask_items, &transparent_items, &num_transparent_items, &additive_items, &num_additive_items);

  // Sort transparent_items back to front
  qsort(transparent_items, num_transparent_items, sizeof(struct RenderItem), compare_render_item_depth);

  // Draw RenderItem arrays in order: opaque, mask, transparent, additive
  glDisable(GL_BLEND);
  draw_render_items(opaque_items, num_opaque_items, &context);

  draw_render_items(mask_items, num_mask_items, &context);
  // printf("Rendered %d mask meshes\n", num_mask_items);

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  draw_render_items(transparent_items, num_transparent_items, &context);
  // printf("Rendered %d transparent meshes\n", num_transparent_items);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  draw_render_items(additive_items, num_additive_items, &context);
  // printf("Rendered %d additive meshes\n", num_additive_items);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Free allocated RenderItem arrays (optimize because this seems like a lot more work than I should have to do for this every single frame)
  free(opaque_items);
  free(mask_items);
  free(transparent_items);
  free(additive_items);

  // Draw skybox
  skybox_render(scene->skybox, &context);

  // Draw physics debug volumes
  if (scene->physics_debug_mode){
    physics_debug_render(scene->physics_world, &context);
  }
}

// TODO refactor to free scene graph
void scene_free(struct Scene *scene){
  // Free models
  for (int i = 0; i < scene->num_models; i++){
    free(scene->models[i]);
  }
  // Free shaders
  for (int i = 0; i < scene->num_shaders; i++){
    glDeleteProgram(scene->shaders[i]->ID);
    free(scene->shaders[i]);
  }

  // Free scene graph
  scene_remove_scene_node(scene->root_node);

  // Free components
  free(scene->render_components);

  struct AudioManager *audio_manager = engine_get_audio_manager();
  if (!audio_manager){
    fprintf(stderr, "Error: failed to get AudioManager in scene_free\n");
    return;
  }
  for (unsigned int i = 0; i < scene->num_audio_components; i++){
    audio_component_destroy(audio_manager, &scene->audio_components[i]);
  }
  free(scene->audio_components);
  free(scene->camera_components);
  free(scene->player_components);
  for (unsigned int i = 0; i < scene->num_inventory_components; i++){
    free(scene->inventory_components[i].items);
  }
  free(scene->inventory_components);

  // Free entities
  for (unsigned int i = 0; i < scene->num_entities; i++){
    free(scene->entities[i]);
  }
  free(scene->entities);

  // Free skybox
  free(scene->skybox->shader);
  free(scene->skybox);

  // Free lights
  free(scene->lights);

  // Free physics_world
  free(scene->physics_world->static_bodies);
  free(scene->physics_world->dynamic_bodies);
  free(scene->physics_world->player_bodies);

  free(scene);
}

void scene_process_light_json(cJSON *light_json, struct Light *light){
  // Get data (direction, ambient, diffuse, specular)
  cJSON *light_data_json = cJSON_GetObjectItemCaseSensitive(light_json, "data");
  if (!light_data_json){
    fprintf(stderr, "Error: failed to get data object in light json, either invalid or does not exist\n");
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

void scene_process_node_json(
  struct Scene *scene,
  const cJSON *node_json,
  struct SceneNode *current_node,
  struct SceneNode *parent_node,
  struct Model **models,
  Shader **shaders,
  struct PhysicsWorld *physics_world){

  struct Entity *entity = (struct Entity *)calloc(1, sizeof(struct Entity));
  if (!entity){
    fprintf(stderr, "Error: failed to allocate entity in scene_process_node_json\n");
    return;
  }
  uuid_generate(entity->id);
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "position"), entity->position);
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "rotation"), entity->rotation);
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "scale"), entity->scale);

  // Entity type
  cJSON *entity_type_json = cJSON_GetObjectItemCaseSensitive(node_json, "entity_type");
  if (!cJSON_IsNumber(entity_type_json)){
    fprintf(stderr, "Error: failed to get entity type in scene_process_node_json, either invalid or does not exist\n");
    return;
  }

  // Process entity type and appropriate information if present
  EntityType entity_type = cJSON_GetNumberValue(entity_type_json);
  entity->type = entity_type;
  // Right now, there's nothing I need to switch on the type for.
  // switch(entity_type){
  //   case ENTITY_WORLD: {
  //     break;
  //   }
  //   case ENTITY_ITEM: {
  //     break;
  //   }
  //   // TODO More refactoring here when I actually implement multiplayer support.
  //   // Will player information ever need to be serialized in JSON? Possibly.
  //   case ENTITY_PLAYER: {
  //     break;
  //   }
  //   case ENTITY_GROUPING: {
  //     break;
  //   }
  //   case ENTITY_TYPE_COUNT: {
  //     break;
  //   }
  // }
  // Add reference to this entity to scene's entities array and node
  current_node->entity = entity;
  memcpy(current_node->entity_id, entity->id, 16);
  scene->entities[scene->num_entities++] = entity;

  // Process transform
  // Figure out making it work with these only living in the SceneNode, copy to both SceneNode and Entity for now
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "position"), current_node->position);
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "rotation"), current_node->rotation);
  scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "scale"), current_node->scale);

  // Build local and world transforms
  mat4 translation, rotation, scale;
  glm_scale_make(scale, current_node->scale);

  vec3 rotation_radians = {
    glm_rad(current_node->rotation[0]),
    glm_rad(current_node->rotation[1]),
    glm_rad(current_node->rotation[2])
  };
  glm_euler_xyz(rotation_radians, rotation);

  glm_translate_make(translation, current_node->position);

  glm_mat4_identity(current_node->local_transform);
  glm_mat4_mul(rotation, scale, current_node->local_transform);
  glm_mat4_mul(translation, current_node->local_transform, current_node->local_transform);

  // Combine parent transform
  if (parent_node){
    glm_mat4_mul(parent_node->world_transform, current_node->local_transform, current_node->world_transform);
  }
  else {
    glm_mat4_copy(current_node->local_transform, current_node->world_transform);
  }

  // Process PhysicsBody if collider is not null
  cJSON *collider_json = cJSON_GetObjectItemCaseSensitive(node_json, "collider");
  if (!collider_json){
    fprintf(stderr, "Error: failed to get collider object in scene_process_node_json, either invalid or does not exist\n");
    return;
  }
  if (!cJSON_IsNull(collider_json)){
    cJSON *collider_type = cJSON_GetObjectItemCaseSensitive(collider_json, "type");
    if (!cJSON_IsNumber(collider_type)){
      fprintf(stderr, "Error: failed to get type in collider object in scene_process_node_json, either invalid or does not exist\n");
      return;
    }
    cJSON *collider_data_json = cJSON_GetObjectItemCaseSensitive(collider_json, "data");
    if (!collider_data_json){
      fprintf(stderr, "Error: failed to get data in collider object in scene_process_node_json, either invalid or does not exist\n");
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
          fprintf(stderr, "Error: failed to get radius in collider object in scene_process_node_json, either invalid or does not exist\n");
          return;
        }
        sphere.radius = cJSON_GetNumberValue(radius);

        collider.type = type;
        collider.data.sphere = sphere;
        break;
      case COLLIDER_CAPSULE:
        struct Capsule capsule;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "segment_A"), capsule.segment_A);
        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "segment_B"), capsule.segment_B);
        radius = cJSON_GetObjectItemCaseSensitive(collider_data_json, "radius");
        if(!cJSON_IsNumber(radius)){
          fprintf(stderr, "Error: failed to get radius in collider object in scene_process_node_json, either invalid or does not exist\n");
          return;
        }
        capsule.radius = cJSON_GetNumberValue(radius);

        collider.type = type;
        collider.data.capsule = capsule;
        break;
      case COLLIDER_PLANE:
        struct Plane plane;

        scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(collider_data_json, "normal"), plane.normal);

        cJSON *distance = cJSON_GetObjectItemCaseSensitive(collider_data_json, "distance");
        if(!cJSON_IsNumber(distance)){
          fprintf(stderr, "Error: failed to get distance in collider object in scene_process_node_json, either invalid or does not exist\n");
          return;
        }

        plane.distance = cJSON_GetNumberValue(distance);

        collider.type = type;
        collider.data.plane = plane;
        break;
      default:
        break;
    }

    float restitution = 0.0f;
    bool dynamic = false;
    cJSON *dynamic_json = cJSON_GetObjectItemCaseSensitive(collider_json, "dynamic");
    if (!cJSON_IsBool(dynamic_json)){
      fprintf(stderr, "Error: failed to get dynamic bool in collider object in scene_process_node_json, either invalid or does not exist\n");
      return;
    }
    if (cJSON_IsTrue(dynamic_json)){
      cJSON *restitution_json = cJSON_GetObjectItemCaseSensitive(collider_json, "restitution");
      if(!cJSON_IsNumber(restitution_json)){
        fprintf(stderr, "Error: failed to get restitution in collider object in scene_process_node_json\n");
        return;
      }
      restitution = cJSON_GetNumberValue(restitution_json);
      dynamic = true;

      scene_process_vec3_json(cJSON_GetObjectItemCaseSensitive(node_json, "velocity"), current_node->entity->velocity);
    }
    
    // (Assumes every node with a collider also has an entity, maybe this shouldn't always be true?)
    current_node->entity->physics_body = physics_add_body(physics_world, current_node, current_node->entity, collider, restitution, dynamic);
  }

  // Process components
  cJSON *component_list_json = cJSON_GetObjectItemCaseSensitive(node_json, "components");
  if (!cJSON_IsArray(component_list_json)){
    fprintf(stderr, "Error: failed to get components object in scene_process_node_json, invalid or does not exist\n");
    return;
  }

  const cJSON *component_json = NULL;
  cJSON_ArrayForEach(component_json, component_list_json){
    cJSON *component_type_json = cJSON_GetObjectItemCaseSensitive(component_json, "type");
    if (!component_type_json){
      fprintf(stderr, "Error: failed to get component type from component json in scene_process_node_json, invalid or does not exist\n");
      return;
    }
    ComponentType component_type = cJSON_GetNumberValue(component_type_json);
    switch(component_type){
      case COMPONENT_RENDER: {
        cJSON *model_index_json = cJSON_GetObjectItemCaseSensitive(component_json, "model_index");
        cJSON *shader_index_json = cJSON_GetObjectItemCaseSensitive(component_json, "shader_index");
        if (!cJSON_IsNumber(model_index_json)){
          fprintf(stderr, "Error: failed to get model index in scene_process_node_json, invalid or does not exist\n");
          return;
        }
        if (!cJSON_IsNumber(shader_index_json)){
          fprintf(stderr, "Error: failed to get shader index in scene_process_node_json, invalid or does not exist\n");
          return;
        }

        int model_index = (int)cJSON_GetNumberValue(model_index_json);
        int shader_index = (int)cJSON_GetNumberValue(shader_index_json);

        if (model_index >= 0 && shader_index >= 0){
          render_component_create(
            scene,
            entity->id,
            models[model_index],
            shaders[shader_index]
          );
        }
        break;
      }
      case COMPONENT_AUDIO: {
        cJSON *sound_index_json = cJSON_GetObjectItemCaseSensitive(component_json, "sound_index");
        if (!cJSON_IsNumber(sound_index_json)){
          fprintf(stderr, "Error: failed to get sound index in scene_process_node_json, invalid or does not exist\n");
          return;
        }
        struct AudioManager *audio_manager = engine_get_audio_manager();

        int sound_index = cJSON_GetNumberValue(sound_index_json);
        if (sound_index >= 0){
          audio_component_create(scene, entity->id, audio_manager, sound_index);
        }
        break;
      }
      case COMPONENT_ITEM: {
        cJSON *item_id_json = cJSON_GetObjectItemCaseSensitive(component_json, "id");
        if (!cJSON_IsNumber(item_id_json)){
          fprintf(stderr, "Error: failed to get item id in scene_process_node_json, either invalid or does not exist\n");
          return;
        }
        cJSON *item_count_json = cJSON_GetObjectItemCaseSensitive(component_json, "count");
        if (!cJSON_IsNumber(item_count_json)){
          fprintf(stderr, "Error: failed to get item count in scene_process_node_json, either invalid or does not exist\n");
          return;
        }

        entity->item = (struct ItemComponent *)calloc(1, sizeof(struct ItemComponent));
        if (!entity->item){
          fprintf(stderr, "Error: failed to allocate Item in scene_process_node_json\n");
          return;
        }
        entity->item->id = cJSON_GetNumberValue(item_id_json);
        entity->item->count = cJSON_GetNumberValue(item_count_json);
        break;
      }
      default: {
        break;
      }
    }
  }
  
  // Process children
  cJSON *children_json = cJSON_GetObjectItemCaseSensitive(node_json, "children");
  if (!cJSON_IsArray(children_json)){
    fprintf(stderr, "Error: failed to get node children in scene_process_node_json, invalid or does not exist\n");
    return;
  }
  int num_children = cJSON_GetArraySize(children_json);
  current_node->num_children = num_children;

  if (num_children > 0){
    current_node->children = (struct SceneNode **)calloc(num_children, sizeof(struct SceneNode *));

    const cJSON *child_node_json;
    int index = 0;
    cJSON_ArrayForEach(child_node_json, children_json){
      // Allocate node
      current_node->children[index] = (struct SceneNode *)calloc(1, sizeof(struct SceneNode));
      if (!current_node->children[index]){
        fprintf(stderr, "Error: failed to allocate child node in scene_process_node_json\n");
        return;
      }
      
      // Assign parent node
      current_node->children[index]->parent_node = current_node;

      // Recursively process each child node
      scene_process_node_json(scene, child_node_json, current_node->children[index], current_node, models, shaders, physics_world);
      index++;
    }
  }
}

void scene_process_items_json(struct Scene *scene, const cJSON *items_json){
  int num_items = cJSON_GetArraySize(items_json);
  struct ItemRegistry *item_registry = &scene->item_registry;
  item_registry->items = (struct ItemDefinition *)calloc(num_items, sizeof(struct ItemDefinition));
  if (!item_registry->items){
    fprintf(stderr, "Error: failed to allocate ItemRegistry items in scene_process_items_json\n");
    return;
  }
  item_registry->num_items = num_items;

  int index = 0;
  const cJSON *item_json;
  cJSON_ArrayForEach(item_json, items_json){
    struct ItemDefinition *item_definition = &item_registry->items[index];
    // id
    cJSON *item_id_json = cJSON_GetObjectItemCaseSensitive(item_json, "id");
    if (!cJSON_IsNumber(item_id_json)){
      fprintf(stderr, "Error: failed to get item id in scene_process_items_json, either invalid or does not exist\n");
      return;
    }
    int item_id = cJSON_GetNumberValue(item_id_json);
    item_definition->id = item_id;

    // Name
    cJSON *item_name_json = cJSON_GetObjectItemCaseSensitive(item_json, "name");
    if (!cJSON_IsString(item_name_json)){
      fprintf(stderr, "Error: failed to get item name in scene_process_items_json, either invalid or does not exist\n");
      return;
    }
    char *item_name = cJSON_GetStringValue(item_name_json);
    size_t item_name_length = strlen(item_name);
    if (item_name_length >= MAX_ITEM_NAME_LENGTH){
      fprintf(stderr, "Error: item name exceeds max length of %d characters\n", MAX_ITEM_NAME_LENGTH - 1);
      return;
    }
    strncpy(item_definition->name, item_name, MAX_ITEM_NAME_LENGTH - 1);
    item_definition->name[item_name_length] = '\0';

    // Max count
    cJSON *item_max_count_json = cJSON_GetObjectItemCaseSensitive(item_json, "max_count");
    if (!cJSON_IsNumber(item_max_count_json)){
      fprintf(stderr, "Error: failed to get item max_count in scene_process_items_json, either invalid or does not exist\n");
      return;
    }
    int item_max_count = cJSON_GetNumberValue(item_max_count_json);
    item_definition->max_count = item_max_count;

    index++;

    // printf("Item definition id: %d\n", item_definition->id);
    // printf("Item definition name: %s\n", item_definition->name);
    // printf("Item definition max count: %d\n", item_definition->max_count);
  }
}

void scene_player_create(
  struct Scene *scene,
  struct Model *model,
  Shader *shader,
  vec3 position,
  vec3 rotation,
  vec3 scale,
  vec3 velocity,
  vec3 camera_offset,
  float camera_height,
  bool render_entity,
  int inventory_capacity,
  bool is_local){
  // Reallocate PlayerComponent array if full
  if (scene->num_player_components >= scene->max_player_components){
    scene->max_player_components *= 2;
    scene->player_components = realloc(scene->player_components, scene->max_player_components * sizeof(struct PlayerComponent));
    if (!scene->player_components){
      fprintf(stderr, "Error: failed to reallocate scene PlayerComponents in scene_player_create\n");
      return;
    }
  }
  struct PlayerComponent *player = &scene->player_components[scene->num_player_components++];

  // Allocate Entity
  struct Entity *entity = (struct Entity *)calloc(1, sizeof(struct Entity));
  if (!entity){
    fprintf(stderr, "Error: failed to allocate entity in player_init\n");
    return;
  }
  scene->entities[scene->num_entities++] = entity;

  // Assign values to Entity
  uuid_generate(entity->id);
  entity->type = ENTITY_PLAYER;
  // entity->model = model;
  // entity->shader = shader;
  glm_vec3_copy(position, entity->position);
  glm_vec3_copy(rotation, entity->rotation);
  glm_vec3_copy(scale, entity->scale);
  glm_vec3_copy(velocity, entity->velocity);

  // Add to scene graph
  scene_node_create(scene, entity, scene->root_node);

  // Assign values to PlayerComponent
  memcpy(player->entity_id, entity->id, 16);
  glm_vec3_copy(camera_offset, player->camera_offset);
  glm_vec3_copy(camera_offset, player->rotated_offset);
  player->camera_height = camera_height;
  player->render_entity = render_entity;
  player->is_local = is_local;

  // Local player entity uuid
  if (is_local){
    memcpy(scene->local_player_entity_id, entity->id, 16);
  }

  // RenderComponent (could possibly only check model and shader, use render_entity
  // for determining whether to render later so it can be toggled)
  if (render_entity && model && shader){
    render_component_create(scene, player->entity_id, model, shader);
  }

  // CameraComponent
  vec3 cameraPos = {0.0f, 0.0f, 0.0f};
  vec3 cameraUp = {0.0f, 1.0f, 0.0f};
  float yaw = -90.0f;
  float pitch = 0.0f;
  float fov = 90.0f;
  float sensitivity = 0.1f;
  float speed = 2.5f;
  camera_create(scene, entity->id, cameraPos, cameraUp, yaw, pitch, fov, sensitivity, speed);

  // AudioComponent
  struct AudioManager *audio_manager = engine_get_audio_manager();
  if (!audio_manager){
    fprintf(stderr, "Error: failed to get AudioManager in scene_player_create\n");
    return;
  }
  audio_component_create(scene, player->entity_id, audio_manager, 0);

  // Set listener position to camera position
  audio_listener_update(scene, entity->id);
}

void scene_node_update(struct Scene *scene, struct SceneNode *current_node){
  // if (current_node->entity){
    // Update position, rotation
  if (current_node->entity->physics_body){
    glm_vec3_copy(current_node->entity->physics_body->position, current_node->position);
    glm_vec3_copy(current_node->entity->physics_body->position, current_node->entity->position);
    glm_vec3_copy(current_node->entity->physics_body->rotation, current_node->rotation);
    glm_vec3_copy(current_node->entity->physics_body->rotation, current_node->entity->rotation);
  }

  // Build local and world transforms
  glm_mat4_identity(current_node->local_transform);
  glm_translate(current_node->local_transform, current_node->position);
  vec3 rotation_radians = {
    glm_rad(current_node->rotation[0]),
    glm_rad(current_node->rotation[1]),
    glm_rad(current_node->rotation[2])
  };
  mat4 rotation;
  glm_euler_xyz(rotation_radians, rotation);
  glm_mul(current_node->local_transform, rotation, current_node->local_transform);
  glm_scale(current_node->local_transform, current_node->scale);
  // Combine parent transform
  if (current_node->parent_node){
    glm_mat4_mul(current_node->parent_node->world_transform, current_node->local_transform, current_node->world_transform);
  }
  else {
    glm_mat4_copy(current_node->local_transform, current_node->world_transform);
  }

  // Update RenderComponent
  struct RenderComponent *render_component = scene_get_render_component_by_entity_id(scene, current_node->entity_id);
  if (render_component){
    // Copy node world transform to render component
    glm_mat4_copy(current_node->world_transform, render_component->world_transform);
  }

  // Update AudioComponent
  struct AudioComponent *audio_component = scene_get_audio_component_by_entity_id(scene, current_node->entity->id);
  if (audio_component){
    alSource3f(audio_component->source_id, AL_POSITION,
               current_node->entity->position[0],
               current_node->entity->position[1],
               current_node->entity->position[2]);
    ALenum position_error = alGetError();
    if (position_error != AL_NO_ERROR){
      fprintf(stderr, "Error matching Entity audio_source position with entity position in scene_update: %d\n", position_error);
    }
  }

  for (unsigned int i = 0; i < current_node->num_children; i++){
    scene_node_update(scene, current_node->children[i]);
  }
}

void scene_node_create(struct Scene *scene, struct Entity *entity, struct SceneNode *parent_node)
{
  // A node must be created as a child of a parent node
  if (!parent_node){
    fprintf(stderr, "Error: no parent node provided to scene_node_create\n");
    return;
  }

  // Reallocate parent node's children
  // (refactor to include max_children, double size?)
  parent_node->num_children += 1;
  parent_node->children = realloc(parent_node->children, parent_node->num_children);
  if (!parent_node->children){
    fprintf(stderr, "Error: failed to reallocate parent node children in scene_node_create\n");
    return;
  }

  // Initialize SceneNode
  parent_node->children[parent_node->num_children - 1] = (struct SceneNode *)calloc(1, sizeof(struct SceneNode));
  if (!parent_node->children[parent_node->num_children - 1]){
    fprintf(stderr, "Error: failed to allocate SceneNode in scene_node_create\n");
    return;
  }
  struct SceneNode *scene_node = parent_node->children[parent_node->num_children - 1];
  memcpy(scene_node->entity_id, entity->id, 16);
  glm_vec3_copy(entity->position, scene_node->position);
  glm_vec3_copy(entity->rotation, scene_node->rotation);
  glm_vec3_copy(entity->scale, scene_node->scale);

  // Build local and world transforms
  glm_mat4_identity(scene_node->local_transform);
  glm_translate(scene_node->local_transform, scene_node->position);
  vec3 rotation_radians = {
    glm_rad(scene_node->rotation[0]),
    glm_rad(scene_node->rotation[1]),
    glm_rad(scene_node->rotation[2])
  };
  mat4 rotation;
  glm_euler_xyz(rotation_radians, rotation);
  glm_mul(scene_node->local_transform, rotation, scene_node->local_transform);
  glm_scale(scene_node->local_transform, scene_node->scale);
  glm_mat4_mul(parent_node->world_transform, scene_node->local_transform, scene_node->world_transform);

  scene_node->entity = entity;
  scene_node->parent_node = parent_node;
  scene_node->children = NULL;
  scene_node->num_children = 0;
}

// Recursive function to search the scene graph for the node with the entity with the given entity_id
void scene_get_node_by_entity_id(struct SceneNode *current_node, uuid_t entity_id, int *child_index, int *final_child_index, struct SceneNode **dest){
  if (current_node->entity){
    if (uuid_compare(current_node->entity->id, entity_id) == 0){
      *dest = current_node;
      *final_child_index = *child_index;
    }
  }

  for (unsigned int i = 0; i < current_node->num_children; i++){
    *child_index = i;
    scene_get_node_by_entity_id(current_node->children[i], entity_id, child_index, final_child_index, dest);
  }
}

void scene_remove_scene_node(struct SceneNode *scene_node){
  // Remove this node's children
  if (scene_node->children){
    for (unsigned int i = 0; i < scene_node->num_children; i++){
      scene_remove_scene_node(scene_node->children[i]);
    }
  }

  // Remove this node
  free(scene_node);
}

void scene_remove_entity(struct Scene *scene, uuid_t entity_id){
  // Find and remove the correct SceneNode
  struct SceneNode *node_to_remove = NULL;
  int child_index, final_child_index;
  scene_get_node_by_entity_id(scene->root_node, entity_id, &child_index, &final_child_index, &node_to_remove);
  // printf("Address of node_to_remove: %p\n", node_to_remove);
  if (node_to_remove){
    // If the node we want to remove has a parent node,
    // swap and pop its last child to the place of the node we removed
    struct SceneNode *parent_node = node_to_remove->parent_node;
    scene_remove_scene_node(node_to_remove);
    if (parent_node){
      parent_node->children[final_child_index] = parent_node->children[parent_node->num_children - 1];
      parent_node->num_children--;
    }
  }

  // Remove entity and its components
  for(unsigned int i = 0; i < scene->num_entities; i++){
    if (uuid_compare(scene->entities[i]->id, entity_id) == 0){
      // PhysicsBody
      physics_remove_body(scene->physics_world, scene->entities[i]->physics_body);

      // AudioComponent
      scene_remove_audio_component_by_entity_id(scene, entity_id);

      // RenderComponent
      scene_remove_render_component_by_entity_id(scene, entity_id);

      // ItemComponent
      if (scene->entities[i]->item){
        free(scene->entities[i]->item);
      }
      // printf("Successfully removed item component\n");

      // Entity
      free(scene->entities[i]);
      scene->entities[i] = scene->entities[scene->num_entities - 1];
      scene->num_entities--;
      // printf("Successfully removed entity\n");
    }
  }
}

struct Entity *scene_get_entity_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_entities; i++){
    if (uuid_compare(scene->entities[i]->id, entity_id) == 0){
      return scene->entities[i];
    }
  }
  return NULL;
}

struct RenderComponent *scene_get_render_component_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_render_components; i++){
    if (uuid_compare(scene->render_components[i].entity_id, entity_id) == 0){
      return &scene->render_components[i];
    }
  }
  return NULL;
}

struct PlayerComponent *scene_get_player_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_player_components; i++){
    if (uuid_compare(scene->player_components[i].entity_id, entity_id) == 0){
      return &scene->player_components[i];
    }
  }
  return NULL;
}

struct InventoryComponent *scene_get_inventory_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_inventory_components; i++){
    if (uuid_compare(scene->inventory_components[i].entity_id, entity_id) == 0){
      return &scene->inventory_components[i];
    }
  }
  return NULL;
}

struct CameraComponent *scene_get_camera_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_camera_components; i++){
    if (uuid_compare(scene->camera_components[i].entity_id, entity_id) == 0){
      return &scene->camera_components[i];
    }
  }
  return NULL;
}

struct AudioComponent *scene_get_audio_component_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_audio_components; i++){
    if (uuid_compare(scene->audio_components[i].entity_id, entity_id) == 0){
      return &scene->audio_components[i];
    }
  }
  return NULL;
}

bool scene_remove_render_component_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_render_components; i++){
    if (uuid_compare(scene->render_components[i].entity_id, entity_id) == 0){
      scene->render_components[i] = scene->render_components[scene->num_render_components - 1];
      scene->num_render_components--;
      return true;
    }
  }
  return false;
}

bool scene_remove_audio_component_by_entity_id(struct Scene *scene, uuid_t entity_id){
  for (unsigned int i = 0; i < scene->num_audio_components; i++){
    if (uuid_compare(scene->audio_components[i].entity_id, entity_id) == 0){
      struct AudioManager *audio_manager = engine_get_audio_manager();
      if (!audio_manager){
        fprintf(stderr, "Error: failed to get AudioManager in scene_remove_audio_component_by_entity_id\n");
        return false;
      }
      audio_component_destroy(audio_manager, &scene->audio_components[i]);
      scene->audio_components[i] = scene->audio_components[scene->num_audio_components - 1];
      scene->num_audio_components--;
      return true;
    }
  }
  return false;
}
