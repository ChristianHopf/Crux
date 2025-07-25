#pragma once

#include <stdbool.h>
#include <cglm/cglm.h>
#include <assimp/material.h>
#include "scene.h"
#include "skybox.h"
#include "model.h"
#include "entity.h"

// For sorting meshes for multiple rendering passes
struct RenderItem {
  Mesh *mesh;
  struct Model *model;
  Shader *shader;
  mat4 transform;
  float depth;
};

struct RenderContext {
  // Values for shader uniforms (pointers, this struct only exists to pass parameters in a pretty way)
  mat4 *view_ptr;
  mat4 *projection_ptr;
  struct Light *light_ptr;
  vec3 *camera_position_ptr;
};

// Rendering
void draw_render_items(struct RenderItem *render_items, unsigned int num_render_items, struct RenderContext *context);
void entity_render(struct Entity *entity, struct RenderContext *context);
void skybox_render(struct Skybox *skybox, struct RenderContext *context);

// RenderItems functions
void sort_render_items(
  struct Entity *entities,
  unsigned int num_entities,
  vec3 camera_pos,
  struct RenderItem **opaque_items, unsigned int *num_opaque_items,
  struct RenderItem **mask_items, unsigned int *num_mask_items,
  struct RenderItem **transparent_items, unsigned int *num_transparent_items,
  struct RenderItem **additive_items, unsigned int *num_additive_items);
int compare_render_item_depth(const void *a, const void *b);
