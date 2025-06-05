#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "material.h"

void material_load_textures(int *model, struct aiMaterial *mat, const struct aiScene *scene){
  for (unsigned int i = 0; i < mat->mNumProperties){
    struct aiMaterialProperty *property = mat->mProperties[i];
    
    // If this is a texture property
    if (property->mSemantic > 0){
      enum aiTextureType type = (enum aiTextureType)property->mSemantic;
      unsigned int texture_index = property->mIndex;
      switch(type){

      }
    }
  }
}
