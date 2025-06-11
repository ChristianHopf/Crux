#pragma once

#include "scene.h"
#include "level.h"
#include "entity.h"

struct RenderContext {
  mat4 view;
  mat4 projection;
  struct Light *light;
  vec3 camera_position;
}


void level_render(struct Level *level, struct RenderContext *context);
void entity_render(struct Entity *entity, struct RenderContext *context);
