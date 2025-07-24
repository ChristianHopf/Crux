#pragma once

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "audio_manager.h"
#include "camera.h"
#include "entity.h"
#include "utils.h"

struct Player {
  struct Camera *camera;
  struct Entity *entity;
  // If camera position syncs with entity position,
  // the camera points at the player's "feet."
  // If the camera_offset is updated by its magnitude and
  // the camera's yaw and pitch, it loses that height offset I want.
  float camera_height;
  vec3 camera_offset;
  bool is_grounded;
};

struct PlayerManager {
  struct Player player;
};

void player_init(struct Player *player, struct Model *model, Shader *shader);
void player_process_keyboard_input(struct Player *player, CameraDirection camera_direction, float delta_time);
void player_process_mouse_input(struct Player *player, float xoffset, float yoffset);
void player_jump(struct Player *player);
void player_update(struct Player *player, float delta_time);
