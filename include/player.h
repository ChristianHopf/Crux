#pragma once

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "audio_manager.h"
#include "camera.h"
#include "entity.h"

struct Player {
  struct Camera *camera;
  struct Entity *entity;
  vec3 camera_offset;
  bool is_grounded;
};

struct PlayerManager {
  struct Player player;
};

void player_init(struct Player *player, struct Model *model, Shader *shader);
void player_process_keyboard_input(struct Player *player, CameraDirection camera_direction, float delta_time);
void player_jump(struct Player *player);
void player_update(struct Player *player, float delta_time);
