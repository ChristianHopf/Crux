#pragma once

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "audio_manager.h"
#include "camera.h"

struct Player {
  struct Camera *camera;
  struct PhysicsBody *physics_body;
  vec3 velocity;
  bool is_grounded;
};

void player_init(struct Player *player);
void player_jump(struct Player *player);
void player_update(struct Player *player, float delta_time);
