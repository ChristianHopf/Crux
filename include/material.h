#pragma once

// #include <glad/glad.h>
#include <cglm/cglm.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include <string.h>
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
  vec3 diffuse_color;
  vec4 specular;
  vec3 emissive_color;
  // mat.gltf.blendMode can be OPAQUE (0), MASK (1), BLEND (2), or ADDITIVE (3)
  // Maybe make this a char pointer so it makes more sense, or an enum
  int blend_mode;
  float alpha_cutoff;
  enum aiShadingMode shading_mode;
  // Diffuse opacity
  float opacity;
  float shininess;

  bool has_diffuse;
  bool has_emissive;
};

// Load all textures in a model's material
void material_load_textures(struct Material *mat, struct aiMaterial *ai_mat, const struct aiScene *scene, const char *directory);
GLuint material_load_texture(const char *path, enum aiTextureType type);
GLuint material_load_embedded_texture(const char *path, const struct aiScene *scene);
GLuint check_loaded_texture(const char *path);
void add_loaded_texture(const char *path, GLuint texture_id);
