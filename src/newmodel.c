#include "newmodel.h"
#include "utils.h"
#include <stdbool.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

bool model_load(Model *model, const char *path){
  const struct aiScene* scene = aiImportFile("model.gltf", aiProcess_Triangulate | aiProcess_GenNormals | aiProcess_CalcTangentSpace);
  if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) {
    printf("Error: failed to get scene\n");
    return false;
  }

  // Allocate memory for meshes
  Mesh *meshes = (Mesh *)malloc(scene->mNumMeshes * sizeof(Mesh));
  if (!meshes){
    printf("Error: failed to allocate meshes\n");
    return false;
  }
  // Process root node
  model_process_node(Model *model, scene->mRootNode, scene);
}

void model_draw(Model *model);

void model_process_node(Model *model, struct aiNode *node, struct aiScene *scene){}

void model_process_mesh(struct aiMesh *ai_mesh, Mesh *dest_mesh){}
