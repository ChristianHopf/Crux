#include <cglm/cglm.h>
#include <cglm/io.h>
#include <cglm/mat4.h>
#include <cglm/vec2.h>
#include <stb_image/stb_image.h>
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "model.h"
#include "physics/aabb.h"
#include "utils.h"
#include "material.h"

bool model_load(struct Model *model, const char *path){
  const struct aiScene* scene = aiImportFile(path, aiProcess_GenBoundingBoxes | aiProcessPreset_TargetRealtime_Fast);

  if(!scene || !scene->mRootNode || !scene->mMeshes || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    printf("ERROR::ASSIMP:: %s\n", aiGetErrorString());
    return false;
  }

  // Get directory substring
  char *slash = strrchr(path, '/');
  if(slash){
    size_t directory_length = slash - path;
    model->directory = (char *)malloc(directory_length + 1);
    if (!model->directory){
      printf("Error: failed to allocate model directory in model_load\n");
      return false;
    }
    strncpy(model->directory, path, directory_length);
    model->directory[directory_length] = 0;
  } else {
    model->directory = strdup(".");
  }

  // Allocate memory for meshes
  model->num_meshes = scene->mNumMeshes;
  if (model->num_meshes == 0){
    printf("Error: found 0 meshes in scene\n");
    return false;
  }
  model->meshes = (Mesh *)malloc(scene->mNumMeshes * sizeof(Mesh));
  if (!model->meshes){
    printf("Error: failed to allocate meshes\n");
  return false;
  }

  // Materials
  model->materials = (struct Material *)calloc(scene->mNumMaterials, sizeof(struct Material));
  if (!model->materials){
    printf("Error: failed to allocate struct Materials in model_load\n");
    return false;
  }
  for (unsigned int i = 0; i < scene->mNumMaterials; i++){
    struct aiMaterial *mat = scene->mMaterials[i];

    glm_vec3_copy(model->materials[i].ambient, (vec3){0.2f, 0.2f, 0.2f});
    glm_vec3_copy(model->materials[i].diffuse_color, (vec3){0.8f, 0.8f, 0.8f});
    glm_vec3_copy(model->materials[i].specular, (vec3){1.0f, 1.0f, 1.0f});
    model->materials[i].shininess = 32.0f;

    material_load_textures(&model->materials[i], mat, scene, model->directory);
  }

  // Process the root node's meshes, build AABB
  unsigned int model_mesh_index = 0;
  struct aiMatrix4x4 parent_transform;
  aiIdentityMatrix4(&parent_transform);
  model_process_node(model, scene->mRootNode, scene, parent_transform, &model_mesh_index);

  aiReleaseImport(scene);
  return true;
}

void model_process_node(struct Model *model, struct aiNode *node, const struct aiScene *scene, struct aiMatrix4x4 parent_transform, unsigned int *index){
  // Apply parent node's transformation to this node,
  // then pass that transformation to this node's children
  struct aiMatrix4x4 current_transform = parent_transform;
  aiMultiplyMatrix4(&current_transform, &node->mTransformation);

  // Process each of this node's meshesMore actions
  // The scene has an array of meshes.
  // Each node has an array of ints which are indices to its mesh in the scene's mesh array.
  // The int at position i of this node's mMeshes is the index of its mesh in the scene's mesh array
 for(unsigned int i = 0; i < node->mNumMeshes; i++){
    struct aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
    
    // Process this mesh and update this node's AABB by the mesh's AABB
    model_process_mesh(ai_mesh, scene, current_transform, &model->meshes[*index]);
    (*index)++;
  }

  // Process this node's children
  for (unsigned int i = 0; i < node->mNumChildren; i++){
    model_process_node(model, node->mChildren[i], scene, current_transform, index);
  }
}

void model_process_mesh(struct aiMesh *ai_mesh, const struct aiScene *scene, struct aiMatrix4x4 node_transform, Mesh *dest_mesh){

  // Allocate memory for vertices
  Vertex *vertices = (Vertex *)malloc(ai_mesh->mNumVertices * sizeof(Vertex));
  if (!vertices){
    printf("Error: failed to allocate vertices in model_process_mesh\n");
  }

  // Vertices allocated, get mat4 for transforming vertices
  mat4 node_transform_mat4;
  aiMatrix4x4_to_mat4(&node_transform, node_transform_mat4);

  // Process vertices
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++){
    // Position (transformed to model space)
    vec4 pos = {ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z, 1.0};
    vec4 transformed_pos;
    glm_mat4_mulv(node_transform_mat4, pos, transformed_pos);
    memcpy(vertices[i].position, transformed_pos, sizeof(float) * 3);

    // Normal
    if (ai_mesh->mNormals){
      memcpy(vertices[i].normal, &ai_mesh->mNormals[i], sizeof(float) * 3);
    }else{
      memset(vertices[i].normal, 0, sizeof(float) * 3);
    }

    // Tex_Coord
    if (ai_mesh->mTextureCoords[0]){
      glm_vec2_copy((vec2){ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y}, vertices[i].tex_coord);
    } else{
      glm_vec2_copy((vec2){0.0f, 0.0f}, vertices[i].tex_coord);
    }

    // Tangent, Bitangent (for normal mapping)
    if (ai_mesh->mTangents){
      memcpy(vertices[i].tangent, &ai_mesh->mTangents[i], sizeof(float) * 3);
    }
    if (ai_mesh->mBitangents){
      memcpy(vertices[i].bitangent, &ai_mesh->mBitangents[i], sizeof(float) * 3);
    }
  }

  // Allocate memory for indices
  unsigned int *indices = (unsigned int *)malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));
  if (!indices){
    printf("Error: failed to allocate indices in model_process_mesh\n");
  }

  // Process indices
  unsigned int num_indices = 0;
  for(unsigned int i = 0; i < ai_mesh->mNumFaces; i++){
    struct aiFace face = ai_mesh->mFaces[i];
    for(unsigned int j = 0; j < face.mNumIndices; j++){
      indices[num_indices] = face.mIndices[j];
      num_indices++;
    }
  }
  dest_mesh->num_indices = num_indices;
  dest_mesh->material_index = ai_mesh->mMaterialIndex;

  // Get center from AABB min and max vectors, apply node space transform
  struct aiVector3D min = ai_mesh->mAABB.mMin;
  struct aiVector3D max = ai_mesh->mAABB.mMax;

  dest_mesh->center[0] = (min.x + max.x) * 0.5f;
  dest_mesh->center[1] = (min.y + max.y) * 0.5f;
  dest_mesh->center[2] = (min.z + max.z) * 0.5f;
  vec3 node_space_center;
  glm_mat4_mulv3(node_transform_mat4, dest_mesh->center, 1.0f, node_space_center);
  glm_vec3_copy(node_space_center, dest_mesh->center);

  // Bind vertex buffers and buffer vertex data
  glGenBuffers(1, &dest_mesh->VBO);
  glGenBuffers(1, &dest_mesh->EBO);
  glGenVertexArrays(1, &dest_mesh->VAO);

  // Bind vertex array
  glBindVertexArray(dest_mesh->VAO);

  // Bind element buffers and buffer indices data
  glBindBuffer(GL_ARRAY_BUFFER, dest_mesh->VBO);
  glBufferData(GL_ARRAY_BUFFER, ai_mesh->mNumVertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, dest_mesh->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, num_indices * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Configure attribute pointers
  // Position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
  // Normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  // Tex_coord
  glEnableVertexAttribArray(2);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coord));
  // Tangent
  glEnableVertexAttribArray(3);
  glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tangent));
  // Bitangent
  glEnableVertexAttribArray(4);
  glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, bitangent));

  glBindVertexArray(0);

  // Pretty sure I just don't need to keep these since I have the VBO and EBO ids.
  free(vertices);
  free(indices);
}

void model_draw(struct Model *model, Shader *shader){
  // For each mesh in the model
  for(unsigned int i = 0; i < model->num_meshes; i++){
    // if (i == 1 || i == 6 || i == 10 || i == 26 || i == 27) continue;

    // My shader has an emissive map uniform, but a mesh might not use
    // an emissive texture. A good enough solution is to just give
    // the shader a bool for whether it should sample whatever emissive
    // texture is currently bound.
    bool has_emissive = false;

    // Only bind textures if this mesh *has* a material.
    // If it doesn't, model->meshes[i].material_index will be negative.
    if (model->meshes[i].material_index >= 0){
      struct Material *mat = &model->materials[model->meshes[i].material_index];

      // Blend mode
      // printf("This mesh's material has blend mode %d\n", mat->blend_mode);
      if (mat->blend_mode == 3 || mat->blend_mode == 2){
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      }
      else{
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      }
      if (mat->blend_mode == 1){
        shader_set_bool(shader, "material.mask", true);
      }
      else{
        shader_set_bool(shader, "material.mask", false);
      }

      // Shading mode
      if (mat->shading_mode == 9) shader_set_bool(shader, "material.unlit", true);
      else shader_set_bool(shader, "material.unlit", false);

      unsigned int diffuse_num = 1;
      unsigned int specular_num = 1;

      // Could simplify binding textures by using an unsigned int array like this,
      // where the nth int corresponds the the number of textures with aiTextureType n
      // - example: texture_counts[aiTextureType_DIFFUSE] = 1
      // Maybe worry about this later.
      // unsigned int *texture_counts = calloc(mat->num_textures * sizeof(unsigned int));
      // texture_counts[mat->textures[j].texture_type_enum]++;

      // Bind textures
      for(unsigned int j = 0; j < mat->num_textures; j++){
                
        // Build uniform string of the form:
        // material.<type><index>
        char texture_uniform[32];
        if (strcmp(mat->textures[j].texture_type, "diffuse") == 0){
          snprintf(texture_uniform, sizeof(texture_uniform), "material.%s%u", mat->textures[j].texture_type, diffuse_num);

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(shader, texture_uniform, j);

          diffuse_num++;
        }
        else if (strcmp(mat->textures[j].texture_type, "specular") == 0){
          snprintf(texture_uniform, sizeof(texture_uniform), "material.%s%u", mat->textures[j].texture_type, specular_num);

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(shader, texture_uniform, j);

          specular_num++;
        }
        else if (strcmp(mat->textures[j].texture_type, "normal") == 0){

          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(shader, "material.normal", j);
        }
        // What I actually need to do is check if this mesh uses an emissive texture,
        // not whether its material has one.
        else if (strcmp(mat->textures[j].texture_type, "emissive") == 0){
          has_emissive = true;
          glActiveTexture(GL_TEXTURE0 + j);
          glBindTexture(GL_TEXTURE_2D, mat->textures[j].texture_id);
          shader_set_int(shader, "material.emissive", j);
          shader_set_bool(shader, "material.has_emissive", has_emissive);
        }
        else{
          has_emissive = false;
          shader_set_bool(shader, "material.has_emissive", has_emissive);
        }
      }
    }

    // Bind its vertex array and draw its triangles
    glBindVertexArray(model->meshes[i].VAO);
    glDrawElements(GL_TRIANGLES, model->meshes[i].num_indices, GL_UNSIGNED_INT, 0);
  }

  // Next mesh will bind its VAO first, so this shouldn't matter. Experiment with and without
  glBindVertexArray(0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void model_free(struct Model *model){
  // Delete vertex arrays and buffers
  for(unsigned int i = 0; i < model->num_meshes; i++){
    glDeleteVertexArrays(1, &model->meshes[i].VAO);
    glDeleteBuffers(1, &model->meshes[i].VBO);
    glDeleteBuffers(1, &model->meshes[i].EBO);
  }
  // Free meshes
  free(model->meshes);
  for(unsigned int i = 0; i < model->num_materials; i++){
    free(model->materials[i].textures);
  }
  free(model->materials);
  free(model);
}
