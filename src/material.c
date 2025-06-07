#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include <stb_image/stb_image.h>
#include "material.h"

void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory){
  // Get number of texture properties to allocate this material's struct Textures
  unsigned int num_texture_properties = 0;
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];
    if (strcmp(property->mKey.data, "$tex.file") == 0){
      num_texture_properties++;
    }
  }
  
  // Skip materials with no texture file properties
  if (num_texture_properties == 0){
    return;
  }

  // Allocate struct Textures
  mat->textures = (struct Texture *)malloc(num_texture_properties * sizeof(struct Texture));
  if(!mat->textures){
    printf("Error: failed to allocate Textures in material_load_textures\n");
    return;
  }

  // Process material properties
  unsigned int texture_index = 0;
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];

    // A property with mKey.data $tex.file is a texture file property
    if (strcmp(property->mKey.data, "$tex.file") == 0){

      // Get texture type and index (used for diffuse1, diffuse2, ...)
      enum aiTextureType type = (enum aiTextureType)property->mSemantic;
      // Don't think I need this
      //unsigned int texture_index = property->mIndex;

      // Texture properties have mType aiPTI_String,
      // which means the bytes in mData are a struct aiString.
      // (probably no need to check mType)
      struct aiString *path = (struct aiString *)property->mData;
      printf("Path data I might not want to free: %s\n", path->data);

      // Load texture, embedded
      if (path->data[0] == '*'){
        printf("Found embedded texture property\n");
        GLuint embedded_texture_id = check_loaded_texture(path->data);
        if (embedded_texture_id == 0){
          printf("Already loaded this texture\n");
          embedded_texture_id = material_load_embedded_texture(path->data, scene);
          add_loaded_texture(path->data, embedded_texture_id);
        }

        // Add texture to material
        mat->textures[texture_index].texture_id = embedded_texture_id;
        mat->textures[texture_index].texture_type = get_texture_type_string(type);
        mat->num_textures++;
        texture_index++;

        // free(path);
        continue;
      }

      // Load texture, not embedded
      if (aiGetMaterialTexture(ai_mat, type, 0, path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
        printf("Error: failed to get texture path in material_load_textures\n");
        return;
      }
      size_t len = strlen(directory) + 1 + strlen(path->data) + 1;
      char *full_texture_path = malloc(len);
      if (!full_texture_path){
        printf("Error: failed to allocate full_texture_path in material_load_textures\n");
        return;
      }
      snprintf(full_texture_path, len, "%s/%s", directory, path->data);

      // Check if the texture is already loaded
      GLuint texture_id = check_loaded_texture(full_texture_path);
      if (texture_id == 0){
        texture_id = material_load_texture(full_texture_path, type);
        add_loaded_texture(full_texture_path, texture_id);
        printf("Loaded texture of type %s\n", aiTextureTypeToString(type));
      }

      free(full_texture_path);

      // Add texture to material
      mat->textures[texture_index].texture_id = texture_id;
      mat->textures[texture_index].texture_type = get_texture_type_string(type);
      mat->num_textures++;
      // printf("assigned texture type: %s to texture with id %d\n", mat->textures[texture_index].texture_type, mat->textures[texture_index].texture_id);
      texture_index++;
    }
  }
}

GLuint material_load_texture(const char *path, enum aiTextureType type){
  printf("Loading texture of type %s\n", aiTextureTypeToString(type));

  int width, height, channels;
  unsigned char* data = stbi_load(path, &width, &height, &channels, 0);
  if (!data){
    printf("Error: Failed to load texture at: %s\n", path);
    return 0;
  }

  // Generate GL textures
  // 
  // Since diffuse textures are typically made in sRGB space,
  // enabling gamma correction means we need to specify GL_SRGB
  // as their internalformat arguments.
  GLenum internal_format, pixel_format;

  printf("Time to set texture formats, texture type: %d\n", type);
  printf("Diffuse texture type: %d\n", aiTextureType_DIFFUSE);

  if (channels == 4){
    if (type == aiTextureType_DIFFUSE | type == aiTextureType_BASE_COLOR){
      internal_format = GL_SRGB_ALPHA;
    }
    else{
      internal_format = GL_RGBA;
    }
    pixel_format = GL_RGBA;
  }
  else if (channels == 3){
    if (type == aiTextureType_DIFFUSE | type == aiTextureType_BASE_COLOR){
      internal_format = GL_SRGB;
    }
    else{
      internal_format = GL_RGB;
    }
    pixel_format = GL_RGB;
  }
  else if (channels == 1){
    internal_format = GL_RED;
    pixel_format = GL_RED;
  }

  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, internal_format, width, height, 0, pixel_format, GL_UNSIGNED_BYTE, data);
  glGenerateMipmap(GL_TEXTURE_2D);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  stbi_image_free(data);
  return texture;
}

GLuint material_load_embedded_texture(const char *path, const struct aiScene *scene){
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

GLuint check_loaded_texture(const char *path){
  // Check if a TextureEntry exists with this texture's path
  for(int i = 0; i < num_loaded_textures; i++){
    if (strncmp(loaded_textures[i].path, path, sizeof(loaded_textures[i].path)) == 0){
      return loaded_textures[i].texture_id;
    }
  }
  return 0;
}

void add_loaded_texture(const char *path, GLuint texture_id){
  if (num_loaded_textures >= MAX_TEXTURES){
    printf("Error: failed to load texture, texture cache full\n");
    return;
  }
  strncpy(loaded_textures[num_loaded_textures].path, path, sizeof(loaded_textures[num_loaded_textures].path) - 1);
  loaded_textures[num_loaded_textures].texture_id = texture_id;
  num_loaded_textures++;
}
