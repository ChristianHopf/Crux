#ifndef SKYBOX_H
#define SKYBOX_H

// #include <glad/glad.h>
#include <stb_image/stb_image.h>
#include "shader.h"

struct Skybox {
  GLuint cubemap_texture_id;
  GLuint cubemapVAO, cubemapVBO;
  Shader *shader;
};


// Generate VBO, VAO, and texture IDs for a cubemap, return as a struct Skybox
// directory is the path to the directory containing 6 texture files of a cubemap
struct Skybox *skybox_create(char *directory);

#endif
