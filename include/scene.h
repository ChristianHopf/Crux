#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include <stdbool.h>
#include "skybox.h"
#include "entity.h"
#include "player.h"

typedef struct {
  vec3 direction;

  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
} Light;

typedef struct {
  Entity *entities;
  int num_entities;
  int max_entities;
  struct Skybox *skybox;
  struct Player player;
  Light *light;
  bool paused;
} Scene;

Scene *scene_create();

void scene_update(Scene *scene, float deltaTime);
void scene_render(Scene *scene);
void scene_pause(Scene *scene);
void scene_free(Scene *scene);

#endif
