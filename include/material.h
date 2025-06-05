#ifndef MATERIAL_H
#define MATERIAL_H

#include <glad/glad.h>
#include <assimp/texture.h>
#include <assimp/material.h>

struct Texture {
  GLuint texture_id;
};

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec4 specular;
  float shininess;
  unsigned int diffuse_texture_id;
  unsigned int specular_texture_id;
};

// Probably need the scene pointer for loading embedded textures
material_load_textures(Model *model, struct aiMaterial *mat, const struct aiScene *scene);
GLuint check_loaded_texture(const char *path);
