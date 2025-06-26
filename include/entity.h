#pragma once

#include <cglm/cglm.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "physics/world.h"
#include "model.h"
#include "shader.h"

struct Entity {
  unsigned int ID;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  struct PhysicsBody *physics_body;
  struct Model *model;
  Shader *shader;
  // Audio
  ALuint audio_source;
  ALfloat audio_position[3];
};
