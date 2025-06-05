#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "material.h"

void material_load_textures(Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene){
  for (unsigned int i = 0; i < mat->mNumProperties; i++){
    struct aiMaterialProperty *property = mat->mProperties[i];
    
    // If this is a texture property
    if (property->mSemantic > 0){
      // Get texture type and index (used for diffuse1, diffuse2, ...)
      enum aiTextureType type = (enum aiTextureType)property->mSemantic;
      unsigned int texture_index = property->mIndex;

      // Is mData a texture path?
      char pathBuf[1024] = {0};
      memcpy(pathBuf, property->mData, property->mDataLength);
      printf("Material texture property has mData: %s\n", pathBuf);
      return;
      // Load textures (for now just testing what mData is)
    }
  }
}
