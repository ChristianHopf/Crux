#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include "material.h"

void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene){
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];
    
    // If this is a texture property;warn("%s");
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
          printf("Successfully loaded new embedded texture at path %s with id %d\n", texture_path, embedded_texture_id);
        }

        // Add texture to material
        // 
      }

      // Load texture, not embedded

      // build full path, other stuff
    }
  }
}

void material_load_embedded_texture(const char *path, const struct aiScene *scene){
  
}

GLuint check_loaded_texture(const char *path){
  
}
