#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "shader.h"

typedef struct {
  char path[512];
  GLuint texture_id;
} TextureEntry;

#define MAX_TEXTURES 128
static TextureEntry loaded_textures[MAX_TEXTURES];
static int num_loaded_textures = 0;

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
} Vertex;

typedef struct {
  GLuint VAO, VBO, EBO;
  unsigned int num_indices;
  unsigned int material_index;
} Mesh;

struct Material {
  vec3 ambient;
  vec3 diffuse;
  vec4 specular;
  float shininess;
  unsigned int diffuse_texture_id;
  unsigned int specular_texture_id;
};

typedef struct {
  Mesh *meshes;
  struct Material *materials;
  unsigned int num_meshes;
  unsigned int num_materials;
  char *directory;
} Model;

bool model_load(Model *model, const char *path);
void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene, unsigned int *index);
void model_process_mesh(Model *model, struct aiMesh *ai_mesh, const struct aiScene *scene, Mesh *dest_mesh);
void model_draw(Model *model, Shader *shader);
void model_free(Model *model);
GLuint model_load_texture_type(Model *model, const struct aiMaterial *material, const struct aiScene *scene, enum aiTextureType type);
GLuint model_load_texture(const char *path);
GLuint model_load_embedded_texture(const char *path, const struct aiScene *scene);
char *get_texture_path(const struct aiMaterial *material, enum aiTextureType type);
GLuint model_check_loaded_texture(const char *path);
void model_add_loaded_texture(const char *path, GLuint texture_id);

#endif
