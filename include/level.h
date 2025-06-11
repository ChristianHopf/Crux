#pragma once

#include <cglm/cglm.h>
#include "model.h"
#include "shader.h"

struct Level {
  vec3 position;
  vec3 rotation;
  vec3 scale;
  struct Model *model;
  Shader *shader;
};
