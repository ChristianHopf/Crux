#ifndef ENTITY_H
#define ENTITY_H
#include "newmodel.h"
#include "shader.h"
#include <cglm/cglm.h>

typedef struct {
  unsigned int ID;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  Model *model;
  Shader *shader;
} Entity;

#endif
