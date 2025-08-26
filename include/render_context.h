#pragma once

#include <cglm/cglm.h>
// #include "scene.h"
// #include "skybox.h"
// #include "model.h"
#include "shader.h"

// For sorting meshes for multiple rendering passes
struct RenderItem {
  struct Mesh *mesh;
  struct Model *model;
  Shader *shader;
  mat4 transform;
  float depth;
};

struct RenderComponent {
  uuid_t entity_id;
  struct Model *model;
  Shader *shader;
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
void skybox_render(struct Skybox *skybox, struct RenderContext *context);

// RenderItems functions
void scene_get_render_item_count(struct SceneNode *scene_node, unsigned int *num_render_items);
void scene_get_render_items(
  struct SceneNode *scene_node,
  vec3 camera_pos,
  struct RenderItem **opaque_items, unsigned int *num_opaque_items,
  struct RenderItem **mask_items, unsigned int *num_mask_items,
  struct RenderItem **transparent_items, unsigned int *num_transparent_items,
  struct RenderItem **additive_items, unsigned int *num_additive_items);
int compare_render_item_depth(const void *a, const void *b);

// RenderComponent
void render_component_create(struct Scene *scene, uuid_t entity_id, struct Model *model, Shader *shader);
