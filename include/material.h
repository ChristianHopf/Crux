#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "utils.h"

typedef struct {
  char path[512];
  GLuint texture_id;
} TextureEntry;

struct Texture {
  GLuint texture_id;
  char *texture_type; // Assigned while loading textures (diffuse, specular, etc)
};

struct Material {
  struct Texture *textures;
  unsigned int num_textures;
  vec3 ambient;
  vec3 diffuse;
  vec4 specular;
  float shininess;
};

// Load all textures in a model's material
void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory);
GLuint material_load_texture(const char *path, enum aiTextureType type);
GLuint material_load_embedded_texture(const char *path, const struct aiScene *scene);
GLuint check_loaded_texture(const char *path);
void add_loaded_texture(const char *path, GLuint texture_id);

#endif
