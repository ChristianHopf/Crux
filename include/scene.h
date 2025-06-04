#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include "entity.h"
#include "camera.h"
#include <stdbool.h>

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
  Camera *camera;
  Light *light;
  bool paused;
} Scene;

Scene *scene_create();

void scene_update(Scene *scene, float deltaTime);
void scene_render(Scene *scene);
void scene_pause(Scene *scene);

#endif
