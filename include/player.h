#ifndef PLAYER_H
#define PLAYER_H

#include "camera.h"

struct Player {
  struct Camera *camera;
};

void player_init(struct Player *player);
//void update_player(struct Player *player, float delta_time);

#endif
