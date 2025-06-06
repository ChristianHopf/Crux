#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <assimp/texture.h>
#include <assimp/material.h>

typedef struct {
  char path[512];
  GLuint texture_id;
} TextureEntry;

#define MAX_TEXTURES 128
static TextureEntry loaded_textures[MAX_TEXTURES];
static int num_loaded_textures = 0;

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

// Load all textures in a model's material
void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory);
GLuint material_load_embedded_texture(const char *path, const struct aiScene *scene);
GLuint material_load_texture(const char *path);
GLuint check_loaded_texture(const char *path);

#endif
