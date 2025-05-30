#include "mesh.h"
#include <GL/gl.h>
#include <stddef.h>
#include <string.h>

Mesh *mesh_create(Vertex *vertices, unsigned int num_vertices, unsigned int *indices, unsigned int num_indices, Texture *textures, unsigned int num_textures){
  Mesh *mesh = (Mesh *)malloc(sizeof(Mesh));
  mesh->vertices = vertices;
  mesh->num_vertices = num_vertices;
  mesh->indices = indices;
  mesh->num_indices = num_indices;
  mesh->textures = textures;
  mesh->num_textures = num_textures;
  mesh_setup(mesh);
  return mesh;
}

void mesh_setup(Mesh *mesh){
  // Generate buffers and vertexarrays
  glGenVertexArrays(1, &mesh->VAO);
  glGenBuffers(1, &mesh->VBO);
  glGenBuffers(1, &mesh->EBO);

  // Bind vertex array
  glBindVertexArray(mesh->VAO);

  // Copy vertices array into vertex buffer
  glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
  glBufferData(GL_ARRAY_BUFFER, mesh->num_vertices * sizeof(Vertex), mesh->vertices, GL_STATIC_DRAW);

  // Copy indices array into elements buffer
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->num_indices * sizeof(unsigned int), mesh->indices, GL_STATIC_DRAW);

  // Set vertex attribute pointers
  // vertex positions
  glEnableVertexAttribArray(0);	
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  // vertex normals
  glEnableVertexAttribArray(1);	
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
  // vertex texture coords
  glEnableVertexAttribArray(2);	
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, tex_coords));

  glBindVertexArray(0);
}

void mesh_draw(Shader *shader, Mesh *mesh){
  unsigned int diffuse_count = 1;
  unsigned int specular_count = 1;
  
  for(unsigned int i = 0; i < mesh->num_textures; i++){
    // Activate texture unit
    glActiveTexture(GL_TEXTURE0 + i);

    char uniform[64];
    char number[16];
    char *name = mesh->textures[i].type;
    if (strcmp(name, "texture_diffuse") == 0){
      snprintf(number, sizeof(number), "%u", diffuse_count++);
      snprintf(uniform, sizeof(uniform), "material.diffuse%s", number);
    }
    else if (strcmp(name, "texture_specular") == 0){
      snprintf(number, sizeof(number), "%u", specular_count++);
      snprintf(uniform, sizeof(uniform), "material.specular%s", number);
    } else{
      continue;
    }
    shader_set_int(shader, uniform, i);
    glBindTexture(GL_TEXTURE_2D, mesh->textures[i].ID);
  }
  glActiveTexture(GL_TEXTURE0);

  // Draw mesh
  glBindVertexArray(mesh->VAO);
  glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, 0);
  glBindVertexArray(0);
}
