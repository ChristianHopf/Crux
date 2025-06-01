#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include "shader.h"

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
  //GLuint texture_id;
} Mesh;

typedef struct {
  Mesh *meshes;
  unsigned int num_meshes;
  Shader *shader;
  char *directory;
} Model;

bool model_load(Model *model, const char *path);
void model_process_mesh(Model *model, struct aiMesh *ai_mesh, Mesh *dest_mesh);
void model_draw(Model *model);
void model_free(Model *model);

#endif
