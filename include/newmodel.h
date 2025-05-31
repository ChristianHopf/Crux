#ifndef NEWMODEL_H
#define NEWMODEL_H

#include <cJSON/cJSON.h>
#include "mesh.h"

typedef struct {
  const char *path;
  unsigned char *data;
  cJSON *json;
} NewModel;

NewModel *newModel_create(const char *path);
void newModel_draw(NewModel *newModel, Shader *shader, Camera *camera);

#endif
