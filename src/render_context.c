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

// First draft algorithm:
// - traverse the graph to get the total number of meshes, allocate RenderItem arrays as above
// - pass allocated arrays as arguments to scene_get_render_items
// - process current node's entity
// - process current node's children
void scene_get_render_item_count(struct SceneNode *scene_node, unsigned int *num_render_items){
  // Get this node's meshes
  if (scene_node->entity){
    *num_render_items += scene_node->entity->model->num_meshes;
  }
  for (unsigned int i = 0; i < scene_node->num_children; i++){
    scene_get_render_item_count(scene_node->children[i], num_render_items);
  }
}

void scene_get_render_items(
  struct SceneNode *scene_node,
  vec3 camera_pos,
  struct RenderItem **opaque_items, unsigned int *num_opaque_items,
  struct RenderItem **mask_items, unsigned int *num_mask_items,
  struct RenderItem **transparent_items, unsigned int *num_transparent_items,
  struct RenderItem **additive_items, unsigned int *num_additive_items)
{
  if (scene_node->entity){
    // Get this node's RenderItems
    struct Entity *entity = scene_node->entity;
    struct Model *model = entity->model;

    for (unsigned int j = 0; j < model->num_meshes; j++){
      Mesh *mesh = &model->meshes[j];

      struct RenderItem render_item;
      render_item.mesh = mesh;
      render_item.model = model;
      render_item.shader = entity->shader;

      // World transform
      glm_mat4_copy(scene_node->world_transform, render_item.transform);

      // Get mesh depth: magnitude of difference between camera pos and mesh center
      vec3 world_mesh_center, difference;
      glm_mat4_mulv3(render_item.transform, mesh->center, 1.0f, world_mesh_center);
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
  
  for (unsigned int i = 0; i < scene_node->num_children; i++){
    scene_get_render_items(scene_node->children[i], camera_pos, opaque_items, num_opaque_items, mask_items, num_mask_items, transparent_items, num_transparent_items, additive_items, num_additive_items);
  }
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

void render_component_create(struct Scene *scene, uuid_t entity_id, struct Model *model, Shader *shader){
  if (scene->num_render_components >= scene->max_render_components){
    scene->max_render_components *= 2;
    scene->render_components = realloc(scene->render_components, scene->max_render_components * sizeof(struct RenderComponent));
    if (!scene->render_components){
      fprintf(stderr, "Error: failed to reallocate scene RenderComponents in render_component_create\n");
      return;
    }
  }

  struct RenderComponent *render_component = &scene->render_components[scene->num_render_components++];
  memcpy(render_component->entity_id, entity_id, 16);
  render_component->model = model;
  render_component->shader = shader;
}
