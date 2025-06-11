#ifndef SKYBOX_H
#define SKYBOX_H

#include <glad/glad.h>
#include <stb_image/stb_image.h>
#include "shader.h"

struct Skybox {
  GLuint cubemap_texture_id;
  GLuint cubemapVAO, cubemapVBO;
  Shader *shader;
};


// Generate VBO, VAO, and texture IDs for a cubemap, return as a struct Skybox
// (Could later take parameters for specific cubemap textures. For now, hardcode everything)
struct Skybox *skybox_create();

#endif
