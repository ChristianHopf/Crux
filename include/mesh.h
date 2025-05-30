#ifndef MESH_H
#define MESH_H

#include "shader.h"
#include <cglm/cglm.h>

typedef struct {
  vec3 position;
  vec3 normal;
  vec2 tex_coords;
} Vertex;

typedef struct {
  unsigned int ID;
  char *type;
  char *path;
} Texture;

typedef struct {
    Texture *textures;
    unsigned int num_textures;
} TextureArray;

typedef struct {
  Vertex *vertices;
  unsigned int num_vertices;
  unsigned int *indices;
  unsigned int num_indices;
  Texture *textures;
  unsigned int num_textures;
  unsigned int VAO, VBO, EBO;
} Mesh;

Mesh *mesh_create(Vertex *vertices, unsigned int num_vertices, unsigned int *indices, unsigned int num_indices, Texture *textures, unsigned int num_textures);

void mesh_draw(Shader *shader, Mesh *mesh);

void mesh_setup(Mesh *mesh);

#endif
