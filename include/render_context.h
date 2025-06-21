#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>
#include "scene.h"
#include "skybox.h"
#include "level.h"
#include "entity.h"

struct RenderContext {
  // Values for shader uniforms (pointers, this struct only exists to pass parameters in a pretty way)
  mat4 *view_ptr;
  mat4 *projection_ptr;
  struct Light *light_ptr;
  vec3 *camera_position_ptr;
};


void level_render(struct Level *level, struct RenderContext *context);
void entity_render(struct Entity *entity, struct RenderContext *context);
void skybox_render(struct Skybox *skybox, struct RenderContext *context);
