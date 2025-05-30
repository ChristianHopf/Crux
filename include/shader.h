#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <cglm/cglm.h>

typedef struct {
	unsigned int ID; // shader program ID
} Shader;

// Creates and compiles shader program with vertex and fragment shader source files
Shader shader_create(const char *vertexPath, const char *fragmentPath);

// Activates shader program
void shader_use(const Shader *shader);

// Sets uniform values
void shader_set_bool(const Shader *shader, const char *name, int value);
void shader_set_int(const Shader *shader, const char *name, int value);
void shader_set_float(const Shader *shader, const char *name, float value);
void shader_set_mat4(const Shader *shader, const char *name, mat4 value);

#endif
