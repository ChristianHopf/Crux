#include <cglm/cglm.h>
#include <assimp/material.h>
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

// Helper to read from file to string
unsigned char *read_file(const char *path){
	// Open file
	FILE *file = fopen(path, "r");
	if (!file){
		printf("Error: file not found at path: %s\n", path);
		return NULL;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate buffer
	char *buffer = (char *)malloc(length + 1);
	if (!buffer){
		printf("Error: failed to allocate buffer\n");
		fclose(file);
		return NULL;
	}

	// Read file
	size_t read_size = fread(buffer, 1, length, file);
	buffer[read_size] = '\0';
	fclose(file);
	return buffer;
}

void print_glm_vec3(float *vector, char *name){
  printf("%s: [%f, %f, %f]\n", name, vector[0], vector[1], vector[2]);
}

char *get_texture_type_string(enum aiTextureType type){
  switch (type) {
    case aiTextureType_DIFFUSE:
      return "diffuse";
    case aiTextureType_SPECULAR:
      return "specular";
    case aiTextureType_HEIGHT:
    case aiTextureType_NORMALS:
      return "normal";
    case aiTextureType_EMISSIVE:
      return "emissive";
    default:
      return "unknown";
  }
}

void print_aiMatrix4x4(struct aiMatrix4x4 *mat) {
    if (!mat) {
        printf("Matrix is NULL\n");
        return;
    }

    printf("aiMatrix4x4:\n");
    printf("[ % .4f % .4f % .4f % .4f ]\n", mat->a1, mat->a2, mat->a3, mat->a4);
    printf("[ % .4f % .4f % .4f % .4f ]\n", mat->b1, mat->b2, mat->b3, mat->b4);
    printf("[ % .4f % .4f % .4f % .4f ]\n", mat->c1, mat->c2, mat->c3, mat->c4);
    printf("[ % .4f % .4f % .4f % .4f ]\n", mat->d1, mat->d2, mat->d3, mat->d4);
}

void aiMatrix4x4_to_mat4(struct aiMatrix4x4 *src, mat4 dest) {
    dest[0][0] = src->a1; dest[1][0] = src->a2; dest[2][0] = src->a3; dest[3][0] = src->a4;
    dest[0][1] = src->b1; dest[1][1] = src->b2; dest[2][1] = src->b3; dest[3][1] = src->b4;
    dest[0][2] = src->c1; dest[1][2] = src->c2; dest[2][2] = src->c3; dest[3][2] = src->c4;
    dest[0][3] = src->d1; dest[1][3] = src->d2; dest[2][3] = src->d3; dest[3][3] = src->d4;
}
