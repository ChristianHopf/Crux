#include "model.h"
#include "utils.h"
#include <cglm/vec2.h>
#include <stb_image/stb_image.h>
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "model.h"
#include "utils.h"

bool model_load(Model *model, const char *path){
  const struct aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_Fast);

  if (!scene || !scene->mRootNode || !scene->mMeshes || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
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

  printf("Model has %d meshes\n", scene->mNumMeshes);
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
  model->materials = (struct Material *)malloc(scene->mNumMaterials * sizeof(struct Material));
  for (unsigned int i = 0; i < scene->mNumMaterials; i++){
    struct aiMaterial *mat = scene->mMaterials[i];

    glm_vec3_copy(model->materials[i].ambient, (vec3){0.2f, 0.2f, 0.2f});
    glm_vec3_copy(model->materials[i].diffuse, (vec3){0.8f, 0.8f, 0.8f});
    glm_vec3_copy(model->materials[i].specular, (vec3){1.0f, 1.0f, 1.0f});
    model->materials[i].shininess = 32.0f;
    model->materials[i].diffuse_texture_id = model_load_texture_type(model, mat, scene, aiTextureType_DIFFUSE);
    model->materials[i].specular_texture_id = model_load_texture_type(model, mat, scene, aiTextureType_SPECULAR);
  }

  // Process the root node
  unsigned int model_mesh_index = 0;
  model_process_node(model, scene->mRootNode, scene, &model_mesh_index);

  aiReleaseImport(scene);
  return true;
}

void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene, unsigned int *index){
  // Process each of this node's meshesMore actions
  // The scene has an array of meshes.
  // Each node has an array of ints which are indices to its mesh in the scene's mesh array.
  // The int at position i of this node's mMeshes is the index of its mesh in the scene's mesh array
 for(unsigned int i = 0; i < node->mNumMeshes; i++){
    struct aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
    model_process_mesh(model, ai_mesh, scene, &model->meshes[*index]);
    (*index)++;
  }

  // Process this node's children
  for (unsigned int i = 0; i < node->mNumChildren; i++){
    model_process_node(model, node->mChildren[i], scene, index);
  }
}

void model_process_mesh(Model *model, struct aiMesh *ai_mesh, const struct aiScene *scene, Mesh *dest_mesh){
  // Allocate memory for vertices
  Vertex *vertices = (Vertex *)malloc(ai_mesh->mNumVertices * sizeof(Vertex));
  if (!vertices){
    printf("Error: failed to allocate vertices in model_process_mesh\n");
  }

  // Process vertices
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++){
    // Position
    memcpy(vertices[i].position, &ai_mesh->mVertices[i], sizeof(float) * 3);

    // Normal
    if (ai_mesh->mNormals){
      memcpy(vertices[i].normal, &ai_mesh->mNormals[i], sizeof(float) * 3);
    }else{
      memset(vertices[i].normal, 0, sizeof(float) * 3);
    }

    // Tex_Coord
    if (ai_mesh->mTextureCoords[0]){
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

  // Allocate memory for indices
  unsigned int *indices = (unsigned int *)malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));
  if (!indices){
    printf("Error: failed to allocate indices in model_process_mesh\n");
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

  dest_mesh->material_index = ai_mesh->mMaterialIndex;

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

void model_draw(Model *model, Shader *shader){
  // For each mesh in the model
  for(unsigned int i = 0; i < model->num_meshes; i++){
    // Only bind textures if this mesh *has* a material.
    // If it doesn't, model->meshes[i].material_index will be negative.
    if (model->meshes[i].material_index >= 0){
      // Bind textures
      // Diffuse
      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, model->materials[model->meshes[i].material_index].diffuse_texture_id);
      shader_set_int(shader, "diffuseMap", 0);

      // Specular
      glActiveTexture(GL_TEXTURE1);
      glBindTexture(GL_TEXTURE_2D, model->materials[model->meshes[i].material_index].specular_texture_id);
      shader_set_int(shader, "specularMap", 1);
    }

    // Bind its vertex array and draw its triangles
    glBindVertexArray(model->meshes[i].VAO);
    glDrawElements(GL_TRIANGLES, model->meshes[i].num_indices, GL_UNSIGNED_INT, 0);
  }

  // Next mesh will bind its VAO first, so this shouldn't matter. Experiment with and without
  glBindVertexArray(0);
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
  free(model);
}

GLuint model_load_texture_type(Model *model, const struct aiMaterial *material, const struct aiScene *scene, enum aiTextureType type){
  // Get texture path
  char *texture_path = get_texture_path(material, type);
    if (!texture_path){
    printf("Error: failed to get texture path of type %s\n", type == aiTextureType_DIFFUSE ? "DIFFUSE" : "SPECULAR");
    return 0;
  }
  // Check if texture is embedded
  if (texture_path[0] == '*'){
    // Check if the texture is already loaded
    GLuint embedded_texture_id = model_check_loaded_texture(texture_path);
    if (embedded_texture_id == 0){
      // Load texture
      embedded_texture_id = model_load_embedded_texture(texture_path, scene);
      model_add_loaded_texture(texture_path, embedded_texture_id);
      printf("Successfully loaded new embedded texture at path %s with id %d\n", texture_path, embedded_texture_id);
    }
    return embedded_texture_id;
  }

  // If texture isn't embedded, the path is different
  char full_texture_path[512];
  snprintf(full_texture_path, sizeof(full_texture_path), "%s/%s", model->directory, texture_path);

  // Check if the texture is already loaded
  GLuint texture_id = model_check_loaded_texture(full_texture_path);
  if (texture_id == 0){
    texture_id = model_load_texture(full_texture_path);
    model_add_loaded_texture(full_texture_path, texture_id);
  }
  free(texture_path);
  return texture_id;
}

// I could probably move this into model_load_texture_type,
// but I might need to just load a texture from a path some time
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

GLuint model_load_embedded_texture(const char *path, const struct aiScene *scene){
  // Texture paths are *0, *1, etc
  int index = atoi(path + 1);
  const struct aiTexture *tex = scene->mTextures[index];

  // Load with aitexture pcData, mWidth, mHeight (texture.h)
  int width, height, channels;
  unsigned char *data = stbi_load_from_memory((char *)tex->pcData, tex->mWidth, &width, &height, &channels, 0);

  // Generate GL textures
  GLenum format;
  if (channels == 4)      format = GL_RGBA;
  else if (channels == 3) format = GL_RGB;
  else if (channels == 1) format = GL_RED;

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

char *get_texture_path(const struct aiMaterial *material, enum aiTextureType type){
  struct aiString path;
  if (aiGetMaterialTexture(material, type, 0, &path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
    return NULL;
  }
  
  return strdup(path.data);
}

GLuint model_check_loaded_texture(const char *path){
  // Check if a TextureEntry exists with this texture's path
  for(int i = 0; i < num_loaded_textures; i++){
    if (strncmp(loaded_textures[i].path, path, sizeof(loaded_textures[i].path)) == 0){
      printf("Texture path %s matches loaded texture with path %s, skipping\n", path, loaded_textures[i].path);
      return loaded_textures[i].texture_id;
    }
  }
  return 0;
}

void model_add_loaded_texture(const char *path, GLuint texture_id){
  if (num_loaded_textures >= MAX_TEXTURES){
    printf("Error: failed to load texture, texture cache full\n");
    return;
  }
  strncpy(loaded_textures[num_loaded_textures].path, path, sizeof(loaded_textures[num_loaded_textures].path) - 1);
  loaded_textures[num_loaded_textures].texture_id = texture_id;
  num_loaded_textures++;
}
