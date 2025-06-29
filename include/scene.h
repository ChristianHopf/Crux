#pragma once

#include <glad/glad.h>
#include <cJSON/cJSON.h>
#include <stdbool.h>
#include "physics/world.h"
#include "skybox.h"
#include "entity.h"
#include "player.h"

struct Light {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

struct Scene {
  struct Entity *static_entities;
  struct Entity *dynamic_entities;
  int num_static_entities;
  int num_dynamic_entities;
  int max_entities;
  struct Skybox *skybox;
  struct Player player;
  struct Light *lights;
  // UBOs
  unsigned int ubo_matrices;
  // Physics
  struct PhysicsWorld *physics_world;
  // // Audio
  // struct AudioStream *music_stream;
  // Options
  bool paused;
  bool physics_debug_mode;
};


struct Scene *scene_init(char *scene_path);
struct Scene *scene_create(bool physics_view_mode);

void scene_update(struct Scene *scene, float deltaTime);
void scene_render(struct Scene *scene);
void scene_pause(struct Scene *scene);
void scene_unpause(struct Scene *scene);
void scene_free(struct Scene *scene);

// JSON processing helpers
void scene_process_meshes_json(cJSON *meshes, struct Model **models, Shader **shaders, struct Entity *entities, struct PhysicsWorld *physics_world, bool dynamic);
void scene_process_light_json(cJSON *light_json, struct Light *light);
void scene_process_vec3_json(cJSON *vec3_json, vec3 dest);
