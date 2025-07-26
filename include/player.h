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
  float camera_distance;
  vec3 camera_offset;
  vec3 rotated_offset;
  bool is_grounded;
  bool render_entity;
};

struct PlayerManager {
  struct Player player;
};

struct Player *player_create(struct Model *model, Shader *shader, vec3 position, vec3 rotation, vec3 scale, vec3 velocity, vec3 camera_offset, float camera_height, bool render_entity);
void player_process_keyboard_input(struct Player *player, CameraDirection camera_direction, float delta_time);
void player_process_mouse_input(struct Player *player, float xoffset, float yoffset);
void player_jump(struct Player *player);
void player_update(struct Player *player, float delta_time);
