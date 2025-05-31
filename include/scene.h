#ifndef SCENE_H
#define SCENE_H

typedef struct {
  Entity *entities;
  int num_entities;
  int max_entities;
  Camera *camera;
} Scene;

Scene *scene_create();

void scene_render(Scene *scene);

#endif
