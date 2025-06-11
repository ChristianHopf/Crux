#include "render_context.h"

void level_render(struct Level *level, struct RenderContext *context){
  shader_use(level->shader);

  // Maybe it doesn't make a lot of sense
  // for a level to have translation or rotation.
  mat4 model;
  glm_mat4_identity(model);
  // // Translate
  glm_translate(model, level->position);
  // // Rotate
  glm_rotate_y(model, glm_rad(level->rotation[1]), model);
  glm_rotate_x(model, glm_rad(level->rotation[0]), model);
  glm_rotate_z(model, glm_rad(level->rotation[2]), model);
  // Scale
  glm_scale(model, level->scale);

  // Set normal matrix uniform
  mat3 transposed_mat3;
  mat3 normal;
  glm_mat4_pick3t(model, transposed_mat3);
  glm_mat3_inv(transposed_mat3, normal);
  shader_set_mat3(level->shader, "normal", normal);

  // Set its model, view, and projection matrix uniforms
  shader_set_mat4(level->shader, "model", model);
  shader_set_mat4(level->shader, "view", context->view);
  shader_set_mat4(level->shader, "projection", context->projection);

  // Lighting uniforms
  shader_set_vec3(level->shader, "dirLight.direction", context->light->direction);
  shader_set_vec3(level->shader, "dirLight.ambient", context->light->ambient);
  shader_set_vec3(level->shader, "dirLight.diffuse", context->light->diffuse);
  shader_set_vec3(level->shader, "dirLight.specular", context->light->specular);

  // Set camera position as viewPos in the fragment shader
  shader_set_vec3(level->shader, "viewPos", context->camera_position);

  // Draw model
  model_draw(level->model, level->shader);
}

void entity_render(struct Entity *entity, struct RenderContext *context){
  shader_use(entity->shader);

  // Get its model matrix
  mat4 model;
  glm_mat4_identity(model);
  // Translate
  glm_translate(model, entity->position);
  // Rotate
  glm_rotate_y(model, glm_rad(entity->rotation[1]), model);
  glm_rotate_x(model, glm_rad(entity->rotation[0]), model);
  glm_rotate_z(model, glm_rad(entity->rotation[2]), model);
  // Scale
  glm_scale(model, entity->scale);

  // Set normal matrix uniform
  mat3 transposed_mat3;
  mat3 normal;
  glm_mat4_pick3t(model, transposed_mat3);
  glm_mat3_inv(transposed_mat3, normal);
  shader_set_mat3(entity->shader, "normal", normal);

  // Set its model, view, and projection matrix uniforms
  shader_set_mat4(entity->shader, "model", model);
  shader_set_mat4(entity->shader, "view", context->view);
  shader_set_mat4(entity->shader, "projection", context->projection);

  // Lighting uniforms
  shader_set_vec3(entity->shader, "dirLight.direction", context->light->direction);
  shader_set_vec3(entity->shader, "dirLight.ambient", context->light->ambient);
  shader_set_vec3(entity->shader, "dirLight.diffuse", context->light->diffuse);
  shader_set_vec3(entity->shader, "dirLight.specular", context->light->specular);

  // Set camera position as viewPos in the fragment shader
  shader_set_vec3(entity->shader, "viewPos", context->camera_position);

  // Draw model
  model_draw(entity->model, entity->shader);

  // Render the model's AABB
  if (context->physics_view_mode){
    AABB_render(&entity->model->aabb, model, context->view, context->projection);
  }
}
