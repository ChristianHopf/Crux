#ifndef UTILS_H
#define UTILS_H

#include <assimp/material.h>
#include <assimp/matrix4x4.h>
#include <cglm/cglm.h>
#include "physics/aabb.h"

// C helpers (strings, etc)
// Read data from file path
unsigned char *read_file(const char *path);

// CGLM helpers
// Print formatted values of a glm_vec3
void print_glm_vec3(vec3 vector, char *name);

// Assimp helpers
char *get_texture_type_string(enum aiTextureType type);
void print_aiMatrix4x4(struct aiMatrix4x4 *mat);
void aiMatrix4x4_to_mat4(struct aiMatrix4x4 *src, mat4 dest);

#endif
