#include "shader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper to read from file to string
static char *read_file(const char *path){
	// Open file
	FILE *file = fopen(path, "r");
	if (!file){
		printf("ERROR::SHADER::FILE_NOT_FOUND: %s\n", path);
		return NULL;
	}

	// Get file size
	fseek(file, 0, SEEK_END);
	long length = ftell(file);
	fseek(file, 0, SEEK_SET);

	// Allocate buffer
	char *buffer = (char *)malloc(length + 1);
	if (!buffer){
		printf("ERROR::SHADER::MEMORY_ALLOCATION_FAILED\n");
		fclose(file);
		return NULL;
	}

	// Read file
	size_t read_size = fread(buffer, 1, length, file);
	buffer[read_size] = '\0';
	fclose(file);
	return buffer;
}

// Helper to compile shaders
static unsigned int compile_shader(unsigned int type, const char *shaderCode){
	// Create and compile shader
	unsigned int shader = glCreateShader(type);
	glShaderSource(shader, 1, &shaderCode, NULL);
	glCompileShader(shader);

	// Check for compilation error
	int success;
	char infoLog[512];
	glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
	if (!success){
		glGetShaderInfoLog(shader, 512, NULL, infoLog);
		printf("ERROR:SHADER::COMPILATION_FAILED (type: %s): %s\n", type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT", infoLog);
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

Shader shader_create(const char *vertexPath, const char *fragmentPath) {
	// Initialize with ID 0
	Shader shader = {0};

	// Read in vertex shader
	char *vertexCode = read_file(vertexPath);
	char *fragmentCode = read_file(fragmentPath);
	if (!vertexCode || !fragmentCode) {
		// If at least one failed, deallocate the other, if any
		if (vertexCode) free(vertexCode);
		if (fragmentCode) free(fragmentCode);
		return shader;
	}
	
	// Compile shaders
	unsigned int vertexShader = compile_shader(GL_VERTEX_SHADER, vertexCode);
	unsigned int fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentCode);
	free(vertexCode);
	free(fragmentCode);
	if (!vertexShader || !fragmentShader){
		if (vertexShader) glDeleteShader(vertexShader);
		if (fragmentShader) glDeleteShader(fragmentShader);
		return shader;
	}

	// Link shaders
	shader.ID = glCreateProgram();
	glAttachShader(shader.ID, vertexShader);
	glAttachShader(shader.ID, fragmentShader);
	glLinkProgram(shader.ID);

	// Check for linking errors
	int success;
	char infoLog[512];
	glGetProgramiv(shader.ID, GL_LINK_STATUS, &success);
	if (!success){
		glGetProgramInfoLog(shader.ID, 512, NULL, infoLog);
		printf("ERROR::SHADER::PROGRAM_LINKING_FAILED: %s\n", infoLog);
		glDeleteProgram(shader.ID);
		shader.ID = 0;
	}

	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	return shader;
}

void shader_use(const Shader *shader){
	if (shader && shader->ID){
		glUseProgram(shader->ID);
	}
}

void shader_set_bool(const Shader *shader, const char *name, int value){
	if (shader && shader->ID) {
        	glUniform1i(glGetUniformLocation(shader->ID, name), value);
    	}
}

void shader_set_int(const Shader *shader, const char *name, int value){
	if (shader && shader->ID) {
        	glUniform1i(glGetUniformLocation(shader->ID, name), value);
    	}
}

void shader_set_float(const Shader *shader, const char *name, float value){
	if (shader && shader->ID) {
        	glUniform1f(glGetUniformLocation(shader->ID, name), value);
    	}
}

void shader_set_mat4(const Shader *shader, const char *name, mat4 value){
	if (shader && shader->ID) {
		glUniformMatrix4fv(glGetUniformLocation(shader->ID, name), 1, GL_FALSE, &value[0][0]);
	}
}
