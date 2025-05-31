#ifndef ENTITY_H
#define ENTITY_H
#include "model.h"
#include "shader.h"
#include <cglm/cglm.h>

typedef struct {
unsigned int ID;
Model *model;
Shader *shader;
} Entity;


#endif
