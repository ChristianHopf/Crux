#include "model.h"
#include "shader.h"
#include "mesh.h"
#include <assimp/material.h>
#include <assimp/mesh.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/types.h>
#include <cglm/vec2.h>
#include <stdbool.h>
#include <string.h>
#include <stb_image/stb_image.h>

Model *model_create(char *path){
  Model *model = (Model *)malloc(sizeof(Model));
  if (!model){
    printf("Error: failed to allocate Model\n");
    return NULL;
  }

  model->meshes = NULL;
  model->num_meshes = 0;
  model->directory = NULL;
  model->textures_loaded = NULL;
  model->num_textures_loaded = 0;
  
  bool loaded = model_load(model, path);
  if (loaded){
    return model;
  }
  else{
    free(model);
    return NULL;
  }
}

void model_draw(Model *model, Shader *shader){
  for (unsigned int i = 0; i < model->num_meshes; i++){
    mesh_draw(shader, &model->meshes[i]);
  }
}

bool model_load(Model *model, char *path){
  const struct aiScene *scene = aiImportFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);
  if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
    printf("ERROR::ASSIMP::%s\n", aiGetErrorString());
  }
  if (scene == NULL){
    printf("Error: importing scene failed\n");
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

  // Allocate meshes and num_meshes based on the scene's number of meshes
  Mesh *meshes = (Mesh *)malloc(scene->mNumMeshes * sizeof(Mesh));
  if (!meshes){
    printf("Error: failed to allocate model meshes in model_process_node");
    free(model->directory);
    aiReleaseImport(scene);
    return false;
  }
  model->meshes = meshes;
  model->num_meshes = scene->mNumMeshes;

  // Process root node
  unsigned int model_mesh_index = 0;
  model_process_node(model, scene->mRootNode, scene, &model_mesh_index);

  aiReleaseImport(scene);
  return true;
}

void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene, unsigned int *index){
  // Process the node's meshes
  for(unsigned int i = 0; i < node->mNumMeshes; i++){
    struct aiMesh *ai_mesh = scene->mMeshes[node->mMeshes[i]];
    Mesh *mesh = model_process_mesh(model, ai_mesh, scene);
    if (!mesh){
      printf("Failed to process mesh in model_process_node\n");
    }
    model->meshes[(*index)++] = *mesh;
  }
  // Process the node's children
  for(unsigned int i = 0; i < node->mNumChildren; i++){
    model_process_node(model, node->mChildren[i], scene, index);
  }
}

Mesh *model_process_mesh(Model *model, struct aiMesh *ai_mesh, const struct aiScene *scene){
  Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
  if (!mesh) {
    printf("Error: failed to allocate Mesh in model_process_mesh\n");
    return NULL;
  }
  //*model_mesh_ptr = (Mesh){0};

  Vertex *vertices;
  unsigned int num_vertices = ai_mesh->mNumVertices;
  unsigned int *indices;
  unsigned int num_indices = (ai_mesh->mNumFaces * 3);
  Texture *textures;
  unsigned int num_textures;

  vertices = malloc(ai_mesh->mNumVertices * sizeof(Vertex));
  if (!vertices){
    printf("Error: failed to allocate model vertices in model_process_mesh\n");
  }
  indices = malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));
  if (!indices){
    printf("Error: failed to allocate model indices in model_process_mesh\n");
  }

  // For each of the mesh's vertices,
  // create a Vertex and push it to the Model's Vertices
  for(unsigned int i = 0; i < ai_mesh->mNumVertices; i++){
    Vertex vertex;
    vec3 temp;
    temp[0] = ai_mesh->mVertices[i].x;
    temp[1] = ai_mesh->mVertices[i].y;
    temp[2] = ai_mesh->mVertices[i].z;
    glm_vec3_copy(temp, vertex.position);
    temp[0] = ai_mesh->mNormals[i].x;
    temp[1] = ai_mesh->mNormals[i].y;
    temp[2] = ai_mesh->mNormals[i].z;
    glm_vec3_copy(temp, vertex.normal);
    if(ai_mesh->mTextureCoords[0]){
      vec2 temp;
      temp[0] = ai_mesh->mTextureCoords[0][i].x;
      temp[1] = ai_mesh->mTextureCoords[0][i].y;
      glm_vec2_copy(temp, vertex.tex_coords);
    }
    vertices[i] = vertex;
  }
  // Process indices
  unsigned int index = 0;
  for(unsigned int i = 0; i < ai_mesh->mNumFaces; i++){
    struct aiFace face = ai_mesh->mFaces[i];
    for(unsigned int j = 0; j < face.mNumIndices; j++){
      indices[index++] = face.mIndices[j];
    }
  }
  // Process material
  if (ai_mesh->mMaterialIndex >= 0){
    struct aiMaterial *material = scene->mMaterials[ai_mesh->mMaterialIndex];
    TextureArray diffuseMaps = model_load_material_textures(model, material, aiTextureType_DIFFUSE, "texture_diffuse");
    TextureArray specularMaps = model_load_material_textures(model, material, aiTextureType_SPECULAR, "texture_specular");

    // Allocate combined textures array
    num_textures = diffuseMaps.num_textures + specularMaps.num_textures;
    textures = (Texture *)malloc(num_textures * sizeof(Texture));
    if (!textures){
      printf("Error: failed to allocate textures for mesh in model_process_load\n");
      return NULL;
    }

    memcpy(textures, diffuseMaps.textures, diffuseMaps.num_textures * sizeof(Texture));
    memcpy(textures + diffuseMaps.num_textures, specularMaps.textures, specularMaps.num_textures * sizeof(Texture));

    // Copy diffuseMaps textures
    //for(unsigned int i = 0; i < diffuseMaps.num_textures; i++){
    //  textures[i] = diffuseMaps.textures[i];
    //}
    // Copy specularMaps textures
    //for(unsigned int i = 0; i < specularMaps.num_textures; i++){
    //  textures[diffuseMaps.num_textures +i] = specularMaps.textures[i];
    //}
  }
  mesh = mesh_create(vertices, num_vertices, indices, num_indices, textures, num_textures);
  return mesh;
}

TextureArray model_load_material_textures(Model *model, struct aiMaterial *mat, enum aiTextureType type, char *type_name){
  // Initialize and allocate TextureArray
  TextureArray result = {NULL, 0};
  unsigned int texture_count = aiGetMaterialTextureCount(mat, type);
  result.textures = malloc(texture_count * sizeof(Texture));
  if (!result.textures){
    printf("Error: failed to allocate TextureArray for textures of type %s in model_load_material_textures\n", type_name);
  }
  result.num_textures = texture_count;

  for(unsigned int i = 0; i < texture_count; i++){
    struct aiString str;
    if (aiGetMaterialTexture(mat, type, i, &str, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS){
      continue;
    }

    // Check if texture has already been loaded
    bool skip = false;
    for(unsigned int j = 0; j < model->num_textures_loaded; j++){
      if (strcmp(model->textures_loaded[j].path, str.data) == 0){
        result.textures[i] = model->textures_loaded[j];
        skip = true;
        break;
      }
    }
    // Only load the texture if it hasn't been loaded yet
    if (!skip){
      Texture texture;
      texture.ID = TextureFromFile(str.data, model->directory);
      texture.type = type_name;
      texture.path = str.data;
      result.textures[i] = texture;
      // Add to loaded_textures, realloc
      model->textures_loaded = (Texture *)realloc(model->textures_loaded, (model->num_textures_loaded + 1) * sizeof(Texture));
      if (!model->textures_loaded){
        printf("Error: failed to reallocate model textures_loaded in model_load_material_textures\n");
      }
      model->textures_loaded[model->num_textures_loaded] = texture;
      model->num_textures_loaded++;
    }
  }
  return result;
}

unsigned int TextureFromFile(const char *path, const char *directory)
{
    char filename[1024];
    snprintf(filename, sizeof(filename), "%s/%s", directory, path);

    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename, &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;
        else
            format = GL_RGB; // fallback

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        printf("Error: failed to load texture at path: %s\n", path);
        stbi_image_free(data);
    }

    return textureID;
}
