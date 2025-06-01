#include "model.h"
#include "utils.h"
#include <cglm/vec2.h>
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool model_load(Model *model, const char *path){
  const struct aiScene* scene = aiImportFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure);

  if (!scene || !scene->mRootNode || !scene->mMeshes || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    printf("Error: failed to get scene\n");
    return false;
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
 
  // Process all meshes
  for(unsigned int i = 0; i < scene->mNumMeshes; i++){
    model_process_mesh(scene->mMeshes[i], &model->meshes[i]);
  }

  const struct aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];
  char* texture_path = get_diffuse_texture_path(material);

  aiReleaseImport(scene);
  return true;
}

void model_process_mesh(Model *model, struct aiMesh *ai_mesh, Mesh *dest_mesh){
  // Get material and texture paths, joined with directory
  const struct aiMaterial* material = scene->mMaterials[ai_mesh->mMaterialIndex];
  char* texture_path = get_diffuse_texture_path(material);
  char full_path[512]; // used to be 1024, 512 is plenty
  snprintf(full_path, sizeof(full)path), "%s/%s", model->directory, texture_path);

  // Allocate vertices and indices
  Vertex *vertices = (Vertex *)malloc(ai_mesh->mNumVertices * sizeof(Vertex));
  if (!vertices){
    printf("Error: failed to allocate vertices in model_process_mesh\n");
  }
  unsigned int *indices = (unsigned int *)malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));
  if (!indices){
    printf("Error: failed to allocate indices in model_process_mesh\n");
  }

  // Process vertices
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++){
    // Copy position vertices to vertex position vector
    memcpy(vertices[i].position, &ai_mesh->mVertices[i], sizeof(float) * 3);
    // Normals
    if (ai_mesh->mNormals){
      memcpy(vertices[i].normal, &ai_mesh->mNormals[i], sizeof(float) * 3);
    }else{
      memset(vertices[i].normal, 0, sizeof(float) * 3);
    }
    // Process UVs
    if (ai_mesh->mTextureCoords[0]){
      //memcpy(vertices[i].tex_coord, &ai_mesh->mTextureCoords[0][i], sizeof(float) * 2);
      // mTextureCoords may have more than 1 channel per vertex, but we only care about
      // the first one for now. Each channel is a vec3 because it may use uvw (for cubemaps or something)
      vec2 temp;
      temp[0] = ai_mesh->mTextureCoords[0][i].x;
      temp[1] = ai_mesh->mTextureCoords[0][i].y;
      glm_vec2_copy(temp, vertices[i].tex_coord);
    } else{
      glm_vec2_copy((vec2){0.0f, 0.0f}, vertices[i].tex_coord);
    }
  }

  // Process indices
  unsigned int index = 0;
  for(unsigned int i = 0; i < ai_mesh->mNumFaces; i++){
    struct aiFace face = ai_mesh->mFaces[i];
    for(unsigned int j = 0; j < face.mNumIndices; j++){
      indices[index++] = face.mIndices[j];
    }
  }
  dest_mesh->num_indices = index;

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
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index * sizeof(unsigned int), indices, GL_STATIC_DRAW);

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

  glBindVertexArray(0);

  free(vertices);
  free(indices);
}

void model_draw(Model *model){
  // For each mesh in the model
  for(unsigned int i = 0; i < model->num_meshes; i++){
    // Bind its vertex array and draw its triangles
    glBindVertexArray(model->meshes[i].VAO);
    glDrawElements(GL_TRIANGLES, model->meshes[i].num_indices, GL_UNSIGNED_INT, 0);
  }
  // Next mesh will bind its VAO first, so this shouldn't matter. Experiment with and without
  glBindVertexArray(0);
}

// Need more of these later, move to a model helpers file?
char *get_diffuse_texture_path(struct aiMaterial *material){
  struct aiString path;
  if (aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
    return NULL;
  }
  return strdup(path.data);
}

void model_free(Model *model){
  // Delete vertex arrays and buffers
  for(unsigned int i = 0; i < model->num_meshes; i++){
    glDeleteVertexArrays(1, &model->meshes[i].VAO);
    glDeleteBuffers(1, &model->meshes[i].VBO);
    glDeleteBuffers(1, &model->meshes[i].EBO);
  }
  // Free meshes
  free(model->meshes);
}
