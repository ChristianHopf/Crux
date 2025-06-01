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
  for(unsigned int i = 0; i < scene->mNumMeshes; ++i){
    model_process_mesh(scene->mMeshes[i], &model->meshes[i]);
  }

  aiReleaseImport(scene);
  return true;
}

void model_draw(Model *model);

void model_process_node(Model *model, struct aiNode *node, struct aiScene *scene){}

void model_process_mesh(struct aiMesh *ai_mesh, Mesh *dest_mesh){
  // Allocate vertices and indices

  // Process vertices

  // Process indices

  // Bind vertex buffers and buffer vertex data

  // Bind element buffers and buffer indices data

  // Configure attribute pointers
}
