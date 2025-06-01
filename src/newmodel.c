#include "newmodel.h"
#include "utils.h"
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool model_load(Model *model, const char *path){
  const struct aiScene* scene = aiImportFile(path,
        aiProcess_Triangulate |
        aiProcess_GenSmoothNormals |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ValidateDataStructure);

  if (!scene || !scene->mRootNode || !scene->HasMeshes() || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    printf("Error: failed to get scene\n");
    return false;
  }

  // Allocate memory for meshes
  Mesh *meshes = (Mesh *)malloc(scene->mNumMeshes * sizeof(Mesh));
  if (!meshes){
    printf("Error: failed to allocate meshes\n");
    return false;
  }
  model->num_meshes = scene->mNumMeshes;
  
  // Process all meshes
  for(unsigned int i = 0; i < scene->mNumMeshes; i++){
    model_process_mesh(scene->mMeshes[i], &model->meshes[i]);
  }

  aiReleaseImport(scene);
  return true;
}

void model_process_mesh(struct aiMesh *ai_mesh, Mesh *dest_mesh){
  // Allocate vertices and indices
  Vertex *vertices = (Vertex *)malloc(ai_mesh->mNumVertices, sizeof(Vertex));
  if (!vertices){
    printf("Error: failed to allocate vertices in model_process_mesh\n");
  }
  unsigned int *indices = (unsigned int *)malloc(ai_mesh->mNumFaces * 3 * sizeof(unsigned int));
  if (!indices){
    printf("Error: failed to allocate indices in model_process_mesh\n");
  }

  // Process vertices
  for (unsigned int i = 0; i < ai_mesh->mNumVertices; i++){
    // Copy position vertices to vertex position vector
    memcpy(vertices[i].position, &ai_mesh->mVertices[i], sizeof(float) * 3);
    if (ai_mesh->HasNormals()){
      memcpy(vertices[i].normal, &ai_mesh->mNormals[i], sizeof(float) * 3);
    }else{
      memset(vertices[i].normal, 0, sizeof(float) * 3);
    }
  }

  // Process indices
  unsigned int index = 0;
  for(unsigned int i = 0; i < ai_mesh->mNumFaces; i++){
    struct aiFace face = ai_mesh->mFaces[i];
    for(unsigned int j = 0; j < face.mNumIndices; j++){
      indices[index++] = face.mIndices[j];
    }
  }
  mesh->num_indices = index;

  // Bind vertex buffers and buffer vertex data
  glGenBuffers(1, &mesh->VBO);
  glGenBuffers(1, &mesh->EBO);
  glGenVertexArrays(1, &mesh->VAO);

  // Bind vertex array
  glBindVertexArray(&mesh->VAO);

  // Bind element buffers and buffer indices data
  glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);
  glBufferData(GL_ARRAY_BUFFER, ai_mesh->mNumvertices * sizeof(Vertex), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, index * sizeof(unsigned int), indices, GL_STATIC_DRAW);

  // Configure attribute pointers
  // Position
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
  // Normal
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));

  glBindVertexArray(0);

  free(vertices);
  free(indices);
}

void model_draw(Model *model){
  // For each mesh in the model
  for(unsigned int i = 0; i < model->num_meshes; i++){
    // Bind its vertex array and draw its triangles
    glBindVertexArray(model->meshes[i].VAO);
    glDrawElements(GL_TRIANGLES, mesh->num_indices, GL_UNSIGNED_INT, 0);
  }
  // Next mesh will bind its VAO first, so this shouldn't matter. Experiment with and without
  glBindVertexArray(0);
}

void model_free(Model *model){
  // Delete vertex arrays and buffers
  for(unsigned int i = 0; i < model->num_meshes; i++){
    glDeleteVertexArrays(1, &model->meshes[i].VAO);
    glDeleteBuffers(1, &model->meshes[i].VBO);
    glDeleteBuffers(1, &model->meshes[i].EBO);
  }
  // Free meshes
  free(model->meshes);
}
