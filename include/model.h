#ifndef MODEL_H
#define MODEL_H

#include "mesh.h"
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <string.h>
#include <stdbool.h>


typedef struct {
  Mesh *meshes;
  unsigned int num_meshes;
  char *directory;
  Texture *textures_loaded;
  unsigned int num_textures_loaded;
} Model;

Model *model_create(char *path);
void model_draw(Model *model, Shader *shader);
bool model_load(Model *model, char *path);
void model_process_node(Model *model, struct aiNode *node, const struct aiScene *scene, unsigned int *index);
Mesh *model_process_mesh(Model *model, struct aiMesh *mesh, const struct aiScene *scene);
TextureArray model_load_material_textures(Model *model, struct aiMaterial *mat, enum aiTextureType type, char *type_name);
unsigned int TextureFromFile(const char *path, const char *directory);

#endif
