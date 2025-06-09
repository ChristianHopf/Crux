#ifndef MODEL_H
#define MODEL_H

#include <glad/glad.h>
#include <cglm/cglm.h>
#include <stdbool.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/cimport.h>
#include <assimp/texture.h>
#include <assimp/material.h>
#include "shader.h"
#include "material.h"
#include "physics/aabb.h"

typedef struct {
    vec3 position;
    vec3 normal;
    vec2 tex_coord;
    vec3 tangent;
    vec3 bitangent;
} Vertex;

typedef struct {
  GLuint VAO, VBO, EBO;
  unsigned int num_indices;
  unsigned int material_index;
} Mesh;

struct Model {
  Mesh *meshes;
  struct Material *materials;
  unsigned int num_meshes;
  unsigned int num_materials;
  char *directory;
  struct AABB *aabb;
};

bool model_load(struct Model *model, const char *path);
struct AABB model_process_node(struct Model *model, struct aiNode *node, const struct aiScene *scene, struct aiMatrix4x4 parent_transform, unsigned int *index);
struct AABB model_process_mesh(struct aiMesh *ai_mesh, const struct aiScene *scene, struct aiMatrix4x4 node_transform, Mesh *dest_mesh);
void model_draw(struct Model *model, Shader *shader);
void model_free(struct Model *model);
GLuint model_load_texture_type(struct Model *model, const struct aiMaterial *material, const struct aiScene *scene, enum aiTextureType type);
// GLuint model_load_texture(const char *path);
// GLuint model_load_embedded_texture(const char *path, const struct aiScene *scene);
// char *get_texture_path(const struct aiMaterial *material, enum aiTextureType type);
// GLuint model_check_loaded_texture(const char *path);
// void model_add_loaded_texture(const char *path, GLuint texture_id);

#endif
