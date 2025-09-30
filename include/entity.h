#pragma once

// #include <glad/glad.h>
#include <cglm/cglm.h>
#include <uuid/uuid.h>
// #include "physics/world.h"
// #include "model.h"
#include "shader.h"
#include "types.h"

struct Entity {
  uuid_t id;
  EntityType type;
  vec3 position;
  vec3 rotation;
  vec3 scale;
  vec3 velocity;
  struct PhysicsBody *physics_body;
  struct Model *model;
  Shader *shader;
  // Audio
  // struct AudioComponent *audio_component;
  struct ItemComponent *item;
};

// void entity_play_sound_effect(struct Entity *entity);
