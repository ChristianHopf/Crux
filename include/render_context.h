#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>
#include "scene.h"
#include "level.h"
#include "entity.h"

struct RenderContext {
  // Values for shader uniforms
  mat4 view;
  mat4 projection;
  struct Light *light;
  vec3 camera_position;

  // Bools
  bool physics_view_mode;
};


void level_render(struct Level *level, struct RenderContext *context);
void entity_render(struct Entity *entity, struct RenderContext *context);
