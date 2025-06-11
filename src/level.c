#include "level.h"

void level_render(struct Level *level, mat4 view, mat4 projection){
  // Use shader, set uniforms
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
  shader_set_mat4(level->shader, "model", model);
  shader_set_mat4(level->shader, "view", view);
  shader_set_mat4(level->shader, "projection", projection);

  // Lighting uniforms
  shader_set_vec3(level->shader, "dirLight.direction", scene->light->direction);
  shader_set_vec3(level->shader, "dirLight.ambient", scene->light->ambient);
  shader_set_vec3(level->shader, "dirLight.diffuse", scene->light->diffuse);
  shader_set_vec3(level->shader, "dirLight.specular", scene->light->specular);

  // Set camera position as viewPos in the fragment shader
  shader_set_vec3(level->shader, "viewPos", scene->player.camera->position);

  // Draw model
  model_draw(level->model, level->shader);
}
