#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include "material.h"

void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene){
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];
    
    // If this is a texture property
    if (property->mSemantic > 0){
      // Get texture type and index (used for diffuse1, diffuse2, ...)
      enum aiTextureType type = (enum aiTextureType)property->mSemantic;
      unsigned int texture_index = property->mIndex;

      // Is mData a texture path?
      char pathBuf[1024] = {0};

      // Texture properties have mType aiPTI_String,
      // which means the bytes in mData are a struct aiString.
      // (probably no need to check mType)
      struct aiString *path = (struct aiString *)property->mData;
      printf("Material texture property has name %s (from mKey)\n", property->mKey.data);
      printf("Material texture property of type %d has path %s\n", type, path->data);

      
      return;
      // Load textures (for now just testing what mData is)
    }
  }
}
