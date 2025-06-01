#include "model.h"
#include "utils.h"
#include <assimp/material.h>
#include <cglm/vec2.h>
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <stb_image/stb_image.h>

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
 
  // Process all meshes
  for(unsigned int i = 0; i < scene->mNumMeshes; i++){
    model_process_mesh(model, scene->mMeshes[i], scene, &model->meshes[i]);
  }

  aiReleaseImport(scene);
  return true;
}

void model_process_mesh(Model *model, struct aiMesh *ai_mesh, const struct aiScene *scene, Mesh *dest_mesh){
  // Get material and texture paths, joined with directory
  const struct aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
  // Get diffuse and specular textures
  char *diffuse_path = get_texture_path(material, aiTextureType_DIFFUSE);
  char *specular_path = get_texture_path(material, aiTextureType_SPECULAR);
  char full_diffuse_path[512]; // used to be 1024, 512 is plenty
  char full_specular_path[512];
  snprintf(full_diffuse_path, sizeof(full_diffuse_path), "%s/%s", model->directory, diffuse_path);
  snprintf(full_specular_path, sizeof(full_specular_path), "%s/%s", model->directory, specular_path);
  // Check if the texture is already loaded
  GLuint diffuse_texture_id = model_check_loaded_texture(full_diffuse_path);
  if (diffuse_texture_id == 0){
    diffuse_texture_id = model_load_texture(full_diffuse_path);
    model_add_loaded_texture(full_diffuse_path, diffuse_texture_id);
  }
  dest_mesh->diffuse_texture_id = diffuse_texture_id;
  free(diffuse_path);

  GLuint specular_texture_id = model_check_loaded_texture(full_specular_path);
  if (specular_texture_id == 0){
    specular_texture_id = model_load_texture(full_specular_path);
    model_add_loaded_texture(full_specular_path, specular_texture_id);
  }
  dest_mesh->specular_textire_id = specular_texture_id;
  free(specular_path);

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
char *get_diffuse_texture_path(const struct aiMaterial *material){
  struct aiString path;
  if (aiGetMaterialTexture(material, aiTextureType_DIFFUSE, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
    return NULL;
  }
  return strdup(path.data);
}

char *get_texture_path(const struct aiMaterial *material, enum aiTextureType type){
  struct aiString path;
  if (aiGetMaterialTexture(material, type, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
    return NULL;
  }
  return strdup(path.data);
}

GLuint model_load_texture(const char *path){
  int width, height, channels;
  unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
  if (!data){
    printf("Error: Failed to load texture at: %s\n", path);
    return 0;
  }

  // Generate GL textures
  GLenum format;
  if (channels == 4)      format = GL_RGBA;
  else if (channels == 3) format = GL_RGB;
  else if (channels == 1) format = GL_RED;
  //else                    format = GL_RGB; // fallback
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  stbi_image_free(data);
  return texture;
}

GLuint model_check_loaded_texture(const char *path){
  // Check if a TextureEntry exists with this texture's path
  for(int i = 0; i < num_loaded_textures; i++){
    if (strncmp(loaded_textures[i].path, path, sizeof(loaded_textures[i].path)) == 0){
      return loaded_textures[i].texture_id;
    }
  }
  return 0;
}

void model_add_loaded_texture(const char *path, GLuint texture_id){
  if (num_loaded_textures >= MAX_TEXTURES){
    printf("Error: texture cache full\n");
    return;
  }
  strncpy(loaded_textures[num_loaded_textures].path, path, sizeof(loaded_textures[num_loaded_textures].path) - 1);
  loaded_textures[num_loaded_textures].texture_id = texture_id;
  num_loaded_textures++;
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
