#ifndef ENTITY_H
#define ENTITY_H
#include <cglm/cglm.h>
#include "model.h"
#include "shader.h"

typedef struct {
  unsigned int ID;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  struct Model *model;
  Shader *shader;
} Entity;

#endif
