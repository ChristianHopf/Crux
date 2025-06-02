#ifndef SCENE_H
#define SCENE_H

#include <glad/glad.h>
#include "entity.h"
#include "camera.h"

typedef struct {
  Entity *entities;
  int num_entities;
  int max_entities;
  Camera *camera;
} Scene;

Scene *scene_create();

void scene_update(Scene *scene, float deltaTime);
void scene_render(Scene *scene);

#endif
