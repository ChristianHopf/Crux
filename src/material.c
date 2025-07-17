#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <assimp/types.h>
#include <stb_image/stb_image.h>
#include "material.h"

#define MAX_TEXTURES 128
static TextureEntry loaded_textures[MAX_TEXTURES];
static int num_loaded_textures = 0;


void material_load(){
  // Refactor to split up processing material properties and loading textures:
  // Load properties (blend mode, shading mode, etc)
  // Load textures
}

void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory){
  // Set defaults (where 0 is not desired)
  mat->opacity = 1.0f;

  // Process material properties
  unsigned int num_texture_properties = 0;
  for (unsigned int i = 0; i < ai_mat->mNumProperties; i++){
    struct aiMaterialProperty *property = ai_mat->mProperties[i];
    // printf("Property string is %s\n", property->mKey.data);

    // Alpha mode
    if (strcmp(property->mKey.data, "$mat.gltf.alphaMode") == 0){
      struct aiString *alphaMode = (struct aiString *)property->mData;
      // printf("$mat.gltf.alphaMode property string is %s\n", alphaMode->data);
      if (strcmp(alphaMode->data, "OPAQUE") == 0){
        mat->blend_mode = 0;
      }
      else if (strcmp(alphaMode->data, "MASK") == 0){
        mat->blend_mode = 1;
      }
      else if (strcmp(alphaMode->data, "BLEND") == 0){
        // Seems like materials that should be additive get marked with BLEND,
        // and you have to determine whether they should be additive by other means
        // (texture filename check below)
        if (mat->blend_mode != 3) mat->blend_mode = 2;
      }
    }

    // Diffuse color
    if (strcmp(property->mKey.data, "$clr.diffuse") == 0){
      struct aiVector3D clr_diffuse = *(struct aiVector3D *)property->mData;
      mat->diffuse_color[0] = clr_diffuse.x;
      mat->diffuse_color[1] = clr_diffuse.y;
      mat->diffuse_color[2] = clr_diffuse.z;
      // printf("$clr.diffuse is %f %f %f\n", clr_diffuse.x, clr_diffuse.y, clr_diffuse.z);
    }

    // Emissive color
    if (strcmp(property->mKey.data, "$clr.emissive") == 0){
      struct aiVector3D clr_emissive = *(struct aiVector3D *)property->mData;
      mat->emissive_color[0] = clr_emissive.x;
      mat->emissive_color[1] = clr_emissive.y;
      mat->emissive_color[2] = clr_emissive.z;
      // printf("$clr.emissive is %f %f %f\n", clr_emissive.x, clr_emissive.y, clr_emissive.z);
    }

    // Opacity
    if (strcmp(property->mKey.data, "$mat.opacity") == 0){
      float mat_opacity = *(float *)property->mData;
      // printf("mat opacity is %f\n", mat_opacity);
      mat->opacity = mat_opacity;
    }

    // Alpha cutoff (should only exist for material with MASK alphaMode)
    if (strcmp(property->mKey.data, "$mat.gltf.alphaCutoff") == 0){
      float alpha_cutoff = *(float *)property->mData;
      // printf("alpha cutoff is %f\n", alpha_cutoff);
      mat->alpha_cutoff = alpha_cutoff;
    }

    // Shading mode (unlit or not)
    if (strcmp(property->mKey.data, "$mat.shadingm") == 0){
      enum aiShadingMode shading_mode = *(int *)property->mData;
      // printf("$mat.shadingm int is %d\n", shading_mode);
      mat->shading_mode = shading_mode;
    }

    // Texture file
    if (strcmp(property->mKey.data, "$tex.file") == 0){
      struct aiString *texPath = (struct aiString *)property->mData;
      
      // Check for "additive" in texture file name (alphaMode does not include additive blending)
      if (strcasestr(texPath->data, "additive") != NULL){
        // printf("Texture path contains the substring \"additive\"!\n");
        mat->blend_mode = 3;
      }
      num_texture_properties++;
    }
  }
  // printf("Material blend mode is %d\n", mat->blend_mode);

  // int blend_mode;
  // if (aiGetMaterialInteger(ai_mat, AI_MATKEY_SHADING_MODEL, &ai_shading_mode) == AI_SUCCESS){
  //   printf("This material has aiShadingMode %d\n", ai_shading_mode);
  // }else{
  //   printf("aiShadingMode %d\n", ai_shading_mode);
  // }
  
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

      struct aiString *path = (struct aiString *)property->mData;

      // Load texture, embedded
      if (path->data[0] == '*'){
        GLuint embedded_texture_id = check_loaded_texture(path->data);
        if (embedded_texture_id == 0){
          embedded_texture_id = material_load_embedded_texture(path->data, scene);
          add_loaded_texture(path->data, embedded_texture_id);
        }

        // Add texture to material
        mat->textures[texture_index].texture_id = embedded_texture_id;
        mat->textures[texture_index].texture_type = get_texture_type_string(type);

        // Set texture bool
        // (I don't like that because of how the logic in this function flows I have to write
        // the same switch twice for the embedded and non-embedded cases, fine for now)
        switch(type){
          case aiTextureType_DIFFUSE: {
            mat->has_diffuse = true;
            break;
          }
          case aiTextureType_EMISSIVE: {
            mat->has_emissive = true;
            break;
          }
        }

        mat->num_textures++;
        texture_index++;

        continue;
      }

      // Load texture, not embedded
      if (aiGetMaterialTexture(ai_mat, type, 0, path, NULL, NULL, NULL, NULL, NULL, NULL) != AI_SUCCESS) {
        printf("Error: failed to get texture path in material_load_textures\n");
        return;
      }

      // Build directory string
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
      }
      free(full_texture_path);

      // Add texture to material
      mat->textures[texture_index].texture_id = texture_id;
      mat->textures[texture_index].texture_type = get_texture_type_string(type);

      // Set texture bool
      switch(type){
        case aiTextureType_DIFFUSE: {
          mat->has_diffuse = true;
          break;
        }
        case aiTextureType_EMISSIVE: {
          mat->has_emissive = true;
          break;
        }
      }

      mat->num_textures++;
      texture_index++;
    }
  }
}

GLuint material_load_texture(const char *path, enum aiTextureType type){
  // printf("Loading texture of type %s\n", aiTextureTypeToString(type));

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

  if (channels == 4){
    if ((type == aiTextureType_DIFFUSE) || (type == aiTextureType_BASE_COLOR)){
      internal_format = GL_SRGB_ALPHA;
    }
    else{
      internal_format = GL_RGBA;
    }
    pixel_format = GL_RGBA;
  }
  else if (channels == 3){
    if ((type == aiTextureType_DIFFUSE) || (type == aiTextureType_BASE_COLOR)){
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
  unsigned char *data = stbi_load_from_memory((unsigned char *)tex->pcData, tex->mWidth, &width, &height, &channels, 0);

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
