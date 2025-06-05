#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <assimp/texture.h>
#include <assimp/material.h>

struct Texture {
  GLuint texture_id;
  enum aiTextureType texture_type;
};

struct Material {
  struct Texture *textures;
  vec3 ambient;
  vec3 diffuse;
  vec4 specular;
  float shininess;
};

// Probably need the scene pointer for loading embedded textures
void material_load_textures(Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene);
GLuint check_loaded_texture(const char *path);

#endif
