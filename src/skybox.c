#include "skybox.h"

// A cubemap skybox doesn't need a Model, or meshes,
// just the vertices of a unit cube
static float cubemapVertices[] = {
    // positions          
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
  };

static char *cubemapFaces[] = {
  "right.jpg",
  "left.jpg",
  "top.jpg",
  "bottom.jpg",
  "front.jpg",
  "back.jpg"
};


struct Skybox *skybox_create(char *directory){

  // Allocate struct Skybox
  struct Skybox *skybox = (struct Skybox *)malloc(sizeof(struct Skybox));
  if (!skybox){
    printf("Error: failed to allocate skybox in skybox_create\n");
    return NULL;
  }

  // Generate VAO, VBO
  GLuint cubemapVAO, cubemapVBO;
  glGenVertexArrays(1, &cubemapVAO);
  glGenBuffers(1, &cubemapVBO);

  // Bind to VAO
  glBindVertexArray(cubemapVAO);

  // Buffer cubemap vertices
  glBindBuffer(GL_ARRAY_BUFFER, cubemapVBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(cubemapVertices), &cubemapVertices, GL_STATIC_DRAW);

  // Configure attribute pointer
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

  // Generate cubemap texture id
  GLuint cubemap_texture_id;
  glGenTextures(1, &cubemap_texture_id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture_id);

  // Load cubemap texture by each face
  stbi_set_flip_vertically_on_load(false);
  int width, height, channels;
  for (unsigned int i = 0; i < 6; i++){

    // Build path string for each face texture
    char facePath[32];
    snprintf(facePath, sizeof(facePath), "%s/%s", directory, cubemapFaces[i]);

    unsigned char *data = stbi_load(facePath, &width, &height, &channels, 0);
    if (!data){
      printf("Error: failed to load cubemap face %s\n", cubemapFaces[i]);
      return NULL;
    }

    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    stbi_image_free(data);
  }
  stbi_set_flip_vertically_on_load(false);

  // Set cubemap texture parameters
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  // Create shader program
  Shader *skyboxShader = shader_create("shaders/cubemap/shader.vs", "shaders/cubemap/shader.fs");
  if (!skyboxShader){
    printf("Error: failed to create skybox shader in skybox_create\n");
    return NULL;
  }

  // Uniform block binding
  unsigned int uniform_block_index = glGetUniformBlockIndex(skyboxShader->ID, "Matrices");
  glUniformBlockBinding(skyboxShader->ID, uniform_block_index, 0);

  skybox->cubemap_texture_id = cubemap_texture_id;
  skybox->cubemapVAO = cubemapVAO;
  skybox->cubemapVBO = cubemapVBO;
  skybox->shader = skyboxShader;
  
  return skybox;
}
