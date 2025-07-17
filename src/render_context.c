#include "render_context.h"

void draw_render_items(struct RenderItem *render_items, unsigned int num_render_items, struct RenderContext *context){

  // For each item
  for (unsigned int i = 0; i < num_render_items; i++){
    struct RenderItem render_item = render_items[i];

    // Use shader, set uniforms
    shader_use(render_item.shader);

    // Set normal matrix uniform
    mat3 transposed_mat3;
    mat3 normal;
    glm_mat4_pick3t(render_item.transform, transposed_mat3);
    glm_mat3_inv(transposed_mat3, normal);
    shader_set_mat3(render_item.shader, "normal", normal);

    // Model, view, and projection matrix uniforms
    shader_set_mat4(render_item.shader, "model", render_item.transform);
    // shader_set_mat4(entity->shader, "view", context->view_ptr);
    // shader_set_mat4(entity->shader, "projection", context->projection_ptr);

    // Lighting uniforms
    shader_set_vec3(render_item.shader, "dirLight.direction", context->light_ptr->direction);
    shader_set_vec3(render_item.shader, "dirLight.ambient", context->light_ptr->ambient);
    shader_set_vec3(render_item.shader, "dirLight.diffuse", context->light_ptr->diffuse);
    shader_set_vec3(render_item.shader, "dirLight.specular", context->light_ptr->specular);

    // Set camera position as viewPos in the fragment shader
    shader_set_vec3(render_item.shader, "viewPos", *context->camera_position_ptr);


    // Bind material info (has_emissive, shading mode (blend mode already handled), textures)
    bool has_emissive = false;

    if (render_item.mesh->material_index >= 0){
      struct Material *mat = &render_item.model->materials[render_item.mesh->material_index];

      // If blend mode is MASK, set mask uniform and alpha cutoff
      // shader_set_bool(render_item.shader, "material.mask", mat->blend_mode == 1);
      if (mat->blend_mode == 1){
        shader_set_bool(render_item.shader, "material.mask", true);
        shader_set_float(render_item.shader, "material.alphaCutoff", mat->alpha_cutoff);
      }
      else shader_set_bool(render_item.shader, "material.mask", false);

      // Shading mode
      if (mat->shading_mode == aiShadingMode_Unlit) shader_set_bool(render_item.shader, "material.unlit", true);
      else shader_set_bool(render_item.shader, "material.unlit", false);

      // Diffuse color
      shader_set_bool(render_item.shader, "material.has_diffuse", mat->has_diffuse);
      shader_set_vec3(render_item.shader, "material.diffuse_color", mat->diffuse_color);

      // Emissive color
      shader_set_bool(render_item.shader, "material.has_emissive", mat->has_emissive);
      shader_set_vec3(render_item.shader, "material.emissive_color", mat->emissive_color);

      // Opacity
      shader_set_float(render_item.shader, "material.opacity", mat->opacity);

      unsigned int diffuse_num = 1;
      unsigned int specular_num = 1;
      
      // Bind textures
      for(unsigned int j = 0; j < mat->num_textures; j++){
        // Build uniform string of the form: material.<type><index>
        char texture_uniform[32];
        if (strcmp(mat->textures[j].texture_type, "diffuse") == 0){
          snprintf(texture_uniform, sizeof(texture_uniform), "material.%s%u", mat->textures[j].texture_type, diffuse_num);

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(render_item.shader, texture_uniform, j);

          diffuse_num++;
        }
        else if (strcmp(mat->textures[j].texture_type, "specular") == 0){
          snprintf(texture_uniform, sizeof(texture_uniform), "material.%s%u", mat->textures[j].texture_type, specular_num);

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(render_item.shader, texture_uniform, j);

          specular_num++;
        }
        else if (strcmp(mat->textures[j].texture_type, "normal") == 0){

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(render_item.shader, "material.normal", j);
        }
        else if (strcmp(mat->textures[j].texture_type, "emissive") == 0){
          has_emissive = true;
          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(render_item.shader, "material.emissive", j);
          shader_set_bool(render_item.shader, "material.has_emissive", has_emissive);
        }
      }
    }

    // Bind its vertex array and draw its triangles
    glBindVertexArray(render_item.mesh->VAO);
    glDrawElements(GL_TRIANGLES, render_item.mesh->num_indices, GL_UNSIGNED_INT, 0);
  }
  glBindVertexArray(0);
}


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
  shader_set_vec3(entity->shader, "viewPos", *context->camera_position_ptr);

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

void sort_render_items(
  struct Entity *entities,
  unsigned int num_entities,
  vec3 camera_pos,
  struct RenderItem **opaque_items, unsigned int *num_opaque_items,
  struct RenderItem **mask_items, unsigned int *num_mask_items,
  struct RenderItem **transparent_items, unsigned int *num_transparent_items,
  struct RenderItem **additive_items, unsigned int *num_additive_items)
{
  // Total number of items is the number of meshes for each Entity
  unsigned int num_items = 0;
  for (unsigned int i = 0; i < num_entities; i++){
    num_items += entities[i].model->num_meshes;
  }

  // Allocate memory for each array of RenderItems (could maybe optimize by somehow figuring out the number for each array beforehand)
  *opaque_items = (struct RenderItem *)malloc(num_items * sizeof(struct RenderItem));
  if (!opaque_items){
    fprintf(stderr, "Error: failed to allocate opaque RenderItems in sort_render_items\n");
    // When rendering, make sure to check whether opaque_items is valid before trying to render them
  }
  *mask_items = (struct RenderItem *)malloc(num_items * sizeof(struct RenderItem));
  if (!mask_items){
    fprintf(stderr, "Error: failed to allocate mask RenderItems in sort_render_items\n");
  }
  *transparent_items = (struct RenderItem *)malloc(num_items * sizeof(struct RenderItem));
  if (!transparent_items){
    fprintf(stderr, "Error: failed to allocate transparent RenderItems in sort_render_items\n");
  }
  *additive_items = (struct RenderItem *)malloc(num_items * sizeof(struct RenderItem));
  if (!additive_items){
    fprintf(stderr, "Error: failed to allocate additive RenderItems in sort_render_items\n");
  }
  // printf("Successfully allocated RenderItem arrays\n");
  // Zero length of each array
  *num_opaque_items = 0;
  *num_mask_items = 0;
  *num_transparent_items = 0;
  *num_additive_items = 0;

  // For each Entity, get its Model
  // For each of the Model's Meshes, create a RenderItem:
  // - pointer to mesh
  // - transform (Entity's model matrix)
  // - get depth as distance from camera to mesh (think I need the center, aiMesh has an aiAABB, can probably get it from that)
  for (unsigned int i = 0; i < num_entities; i++){
    struct Entity *entity = &entities[i];
    struct Model *model = entity->model;

    for (unsigned int j = 0; j < model->num_meshes; j++){
      Mesh *mesh = &model->meshes[j];

      struct RenderItem render_item;
      render_item.mesh = mesh;
      // Would be nice for the mesh struct to have a reference to its Material, rather than just the material index. For now, giving it a reference to its model works.
      render_item.model = model;
      render_item.shader = entity->shader;

      // Compute transform from the Entity's position, rotation, and scale
      glm_mat4_identity(render_item.transform);
      glm_translate(render_item.transform, entity->position);
      vec3 rotation_radians = {glm_rad(entity->rotation[0]), glm_rad(entity->rotation[1]), glm_rad(entity->rotation[2])};
      mat4 rotation;
      glm_euler_xyz(rotation_radians, rotation);
      glm_mul(render_item.transform, rotation, render_item.transform);
      glm_scale(render_item.transform, entity->scale);

      // Get mesh depth: magnitude of difference between camera pos and mesh center
      // print_glm_vec3(camera_pos, "sort_render_items camera position");
      vec3 world_mesh_center, difference;
      glm_mat4_mulv3(render_item.transform, mesh->center, 1.0f, world_mesh_center);
      // glm_vec3_copy(world_mesh_center, mesh->center);
      glm_vec3_sub(camera_pos, world_mesh_center, difference);
      render_item.depth = glm_vec3_norm(difference);

      // Switch on this mesh's material's blend_mode to determine which array to add it to
      switch(model->materials[model->meshes[j].material_index].blend_mode){
        // Opaque
        case 0: {
          (*opaque_items)[(*num_opaque_items)++] = render_item;
          break;
        }
        // Mask
        case 1: {
          (*mask_items)[(*num_mask_items)++] = render_item;
          break;
        }
        // Transparent
        case 2: {
          (*transparent_items)[(*num_transparent_items)++] = render_item;
          break;
        }
        // Additive
        case 3: {
          (*additive_items)[(*num_additive_items)++] = render_item;
          break;
        }
      }
    }
  }

  // Sort transparent_items back to front
  qsort(*transparent_items, *num_transparent_items, sizeof(struct RenderItem), compare_render_item_depth);
  // printf("Successfully sorted %d transparent_items\n", *num_transparent_items);
}

// Helper for sorting transparent RenderItems by depth
int compare_render_item_depth(const void *a, const void *b){
  float depth_A = ((struct RenderItem *)a)->depth;
  float depth_B = ((struct RenderItem *)b)->depth;

  // Descending order: if A is farther away, return -1 (A comes first)
  if (depth_A > depth_B) return -1;
  if (depth_A < depth_B) return 1;
  return 0;
}
