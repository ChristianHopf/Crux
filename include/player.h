#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"

struct Player {
  struct Camera *camera;
  vec3 velocity;
};

void player_init(struct Player *player);
void player_jump(struct Player *player);
void player_update(struct Player *player, float delta_time);

#endif
