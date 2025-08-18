#pragma once

// #include <glad/glad.h>
#include <cJSON/cJSON.h>
#include <stdbool.h>
#include "physics/world.h"
#include "skybox.h"
#include "entity.h"
#include "player.h"
#include "inventory.h"
#include "shader.h"

struct Light {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct SceneNode {
  unsigned int ID;
  mat4 local_transform;
  mat4 world_transform;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  struct Entity *entity;
  struct SceneNode *parent_node;
  struct SceneNode **children;
  unsigned int num_children;
};

struct Scene {
  struct Model **models;
  Shader **shaders;
  int num_models;
  int num_shaders;
  struct SceneNode *root_node;
  struct Entity *player_entities;
  struct Entity **entities;
  unsigned int num_entities;
  int num_player_entities;
  int max_entities;
  struct Skybox *skybox;
  struct Light *lights;
  // UBOs
  unsigned int ubo_matrices;
  // Physics
  struct PhysicsWorld *physics_world;
  // Options
  bool physics_debug_mode;

  // Components
  struct PlayerComponent **player_components;
  unsigned int num_player_components;
  unsigned int max_player_components;
  struct InventoryComponent *inventory_components;
  unsigned int num_inventory_components;
  unsigned int max_inventory_components;

  struct ItemRegistry item_registry;
};


struct Scene *scene_init(char *scene_path);
struct Scene *scene_create(bool physics_view_mode);

void scene_update(struct Scene *scene, float deltaTime);
void scene_node_update(struct SceneNode *current_node);
void scene_get_node_by_entity_id(struct SceneNode *current_node, uuid_t entity_id, int *child_index, int *final_child_index, struct SceneNode **dest);
void scene_remove_entity(struct Scene *scene, uuid_t id);
void scene_remove_scene_node(struct SceneNode *scene_node);
void scene_render(struct Scene *scene);
void scene_free(struct Scene *scene);

// JSON processing helpers
void scene_process_light_json(cJSON *light_json, struct Light *light);
void scene_process_vec3_json(cJSON *vec3_json, vec3 dest);
void scene_process_node_json(struct Scene *scene, const cJSON *node_json, struct SceneNode *current_node, struct SceneNode *parent_node, struct Model **models, Shader **shaders, struct PhysicsWorld *physics_world);
void scene_process_items_json(struct Scene *scene, const cJSON *items_json);

struct PlayerComponent *scene_player_create(
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
  bool is_local);

// Components
struct PlayerComponent *scene_get_player_by_entity_id(struct Scene *scene, uuid_t entity_id);
struct InventoryComponent *scene_get_inventory_by_entity_id(struct Scene *scene, uuid_t entity_id);
