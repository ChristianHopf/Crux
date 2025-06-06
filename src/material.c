#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include "material.h"

void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory){
  // Get number of texture properties to allocate this material's struct Textures
  unsigned int mat_texture_properties = 0;
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];
    if (property->mSemantic > 0){
      num_texture_properties++;
    }
  }

  // Allocate struct Textures
  mat->textures = (struct Texture *)malloc(num_texture_properties * sizeof(struct Texture));
  if(!mat->textures){
    printf("Error: failed to allocate Textures in material_load_textures\n");
    return;
  }

  // Process material properties
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];

    // A property with mSemantic > 0 is a texture
    if (property->mSemantic > 0){

      // Get texture type and index (used for diffuse1, diffuse2, ...)
      enum aiTextureType type = (enum aiTextureType)property->mSemantic;
      unsigned int texture_index = property->mIndex;

      // Texture properties have mType aiPTI_String,
      // which means the bytes in mData are a struct aiString.
      // (probably no need to check mType)
      struct aiString *path = (struct aiString *)property->mData;

      // Load texture, embedded
      if (texture_path[0] == '*'){
        GLuint embedded_texture_id = model_check_loaded_texture(texture_path);
        if (embedded_texture_id == 0){
          embedded_texture_id = model_load_embedded_texture(texture_path, scene);
          model_add_loaded_texture(texture_path, embedded_texture_id);
          printf("Successfully loaded new embedded texture of type %d at path %s with id %d\n", type, texture_path, embedded_texture_id);
        }

        // Add texture to material
        mat->textures[i]->texture_id = embedded_texture_id;
        mat->textures[i]->texture_type = type;
        continue;
      }

      // Load texture, not embedded
      char full_texture_path[512];
      snprintf(full_texture_path, sizeof(full_texture_path), "%s/%s", directory, texture_path);

      // Check if the texture is already loaded
      GLuint texture_id = model_check_loaded_texture(full_texture_path);
      if (texture_id == 0){
        texture_id = model_load_texture(full_texture_path);
        model_add_loaded_texture(full_texture_path, texture_id);
      }

      // Add texture to material
      mat->textures[i]->texture_id = embedded_texture_id;
      mat->textures[i]->texture_type = type;
    }
  }
}

void material_load_embedded_texture(const char *path, const struct aiScene *scene){
  
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
