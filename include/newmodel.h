#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <cJSON/cJSON.h>
#include "mesh.h"

typedef struct {
  const char *path;
  unsigned char *data;
  cJSON *json;
} NewModel;

Model *model_create(const char *path);
void model_draw(Shader *shader, Camera *camera);

#endif
