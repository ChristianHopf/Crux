#include "render_context.h"

void entity_render(struct Entity *entity, struct RenderContext *context){
  shader_use(entity->shader);

  // Get its model matrix
  mat4 model;
  glm_mat4_identity(model);
  // Translate
  glm_translate(model, entity->position);
  // Rotate
  vec3 rotation_radians = {glm_rad(entity->rotation[0]), glm_rad(entity->rotation[1]), glm_rad(entity->rotation[2])};
  mat4 rotation;
  glm_euler_xyz(rotation_radians, rotation);
  glm_mul(model, rotation, model);
  // glm_rotate_x(model, glm_rad(entity->rotation[0]), model);
  // glm_rotate_y(model, glm_rad(entity->rotation[1]), model);
  // glm_rotate_z(model, glm_rad(entity->rotation[2]), model);
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
  // shader_set_mat4(entity->shader, "view", context->view_ptr);
  // shader_set_mat4(entity->shader, "projection", context->projection_ptr);

  // Lighting uniforms
  shader_set_vec3(entity->shader, "dirLight.direction", context->light_ptr->direction);
  shader_set_vec3(entity->shader, "dirLight.ambient", context->light_ptr->ambient);
  shader_set_vec3(entity->shader, "dirLight.diffuse", context->light_ptr->diffuse);
  shader_set_vec3(entity->shader, "dirLight.specular", context->light_ptr->specular);

  // Set camera position as viewPos in the fragment shader
  shader_set_vec3(entity->shader, "viewPos", context->camera_position_ptr);

  // Draw model
  model_draw(entity->model, entity->shader);
}

void skybox_render(struct Skybox *skybox, struct RenderContext *context){
  // Modify depth test to render skybox "at the edge"
  glDepthFunc(GL_LEQUAL);
  shader_use(skybox->shader);

  // Build skybox view matrix, ignoring translation
  // mat3 view_skybox_mat3;
  // mat4 view_skybox;
  // glm_mat4_identity(view_skybox);
  // glm_mat4_pick3(context->view_ptr, view_skybox_mat3);
  // glm_mat4_ins3(view_skybox_mat3, view_skybox);
  //
  // shader_set_mat4(skybox->shader, "view", view_skybox);
  // shader_set_mat4(skybox->shader, "projection", context->projection_ptr);

  // Bind vertex array
  glBindVertexArray(skybox->cubemapVAO);
  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, skybox->cubemap_texture_id);
  // Draw triangles
  glDrawArrays(GL_TRIANGLES, 0, 36);

  // Reset depth test function
  glBindVertexArray(0);
  glDepthFunc(GL_LESS);
}
