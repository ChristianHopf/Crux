#pragma once

#include <glad/glad.h>
#include <stdbool.h>
#include "physics/world.h"
#include "level.h"
#include "skybox.h"
#include "entity.h"
#include "player.h"

struct Light {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

typedef struct {
  struct Entity *entities;
  int num_entities;
  int max_entities;
  struct Level level;
  struct PhysicsWorld *physics_world;
  struct Skybox *skybox;
  struct Player player;
  struct Light *light;
  bool paused;
  bool physics_view_mode;
} Scene;

Scene *scene_create(bool physics_view_mode);

void scene_update(Scene *scene, float deltaTime);
void scene_render(Scene *scene);
void scene_pause(Scene *scene);
void scene_free(Scene *scene);
