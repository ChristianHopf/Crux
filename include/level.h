#pragma once

#include <cglm/cglm.h>
#include "model.h"
#include "shader.h"

struct Level {
  struct Model *model;
  Shader *shader;
};
