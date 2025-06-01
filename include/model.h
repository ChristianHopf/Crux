#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include "shader.h"

typedef struct {
  char path[512];
  GLuint texture_id;
} TextureEntry;

#define MAX_TEXTURES 128
static TextureEntry loaded_textures[MAX_TEXTURES];
static int num_loaded_textures = 0;

typedef enum {
  TEXTURE_TYPE_DIFFUSE,
  TEXTURE_TYPE_NORMAL,
  TEXTURE_TYPE_METALLIC,
  TEXTURE_TYPE_ROUGHNESS,
  TEXTURE_TYPE_OCCLUSION,
  TEXTURE_tYPE_EMISSIVE,
  TEXTURE_TYPE_UNKNOWN
} TextureType;

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
} Vertex;

typedef struct {
  GLuint id;
  TextureType type;
} Texture;

typedef struct {
  GLuint VAO, VBO, EBO;
  unsigned int num_indices;
  GLuint diffuse_texture_id;
  GLuint specular_texture_id;
} Mesh;

typedef struct {
  Mesh *meshes;
  unsigned int num_meshes;
  Shader *shader;
  char *directory;
} Model;

bool model_load(Model *model, const char *path);
void model_process_mesh(Model *model, struct aiMesh *ai_mesh, const struct aiScene *scene, Mesh *dest_mesh);
void model_draw(Model *model);
void model_free(Model *model);
GLuint model_load_texture(const char *path);
char *get_texture_path(const struct aiMaterial *material, enum aiTextureType type);
GLuint model_check_loaded_texture(const char *path);
void model_add_loaded_texture(const char *path, GLuint texture_id);

#endif
