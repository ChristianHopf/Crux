#include "utils.h"
#include <assimp/material.h>
#include <stdio.h>
#include <stdlib.h>

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
