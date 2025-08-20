#pragma once

#include <stdbool.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <uuid/uuid.h>
#include "audio_manager.h"
#include "entity.h"
#include "camera.h"
#include "types.h"
#include "utils.h"
#include "scene.h"

// struct Scene;

struct PlayerComponent {
  uuid_t entity_id;
  struct Camera *camera;
  struct Entity *entity;
  // struct InventoryComponent inventory;
  // If camera position syncs with entity position,
  // the camera points at the player's "feet."
  // If the camera_offset is updated by its magnitude and
  // the camera's yaw and pitch, it loses that height offset I want.
  float camera_height;
  float camera_distance;
  vec3 camera_offset;
  vec3 rotated_offset;
  bool render_entity;
  bool is_local;
};

struct PlayerManager {
  struct PlayerComponent player;
};


struct PlayerComponent *player_create(struct Model *model, Shader *shader, vec3 position, vec3 rotation, vec3 scale, vec3 velocity, vec3 camera_offset, float camera_height, bool render_entity, int inventory_capacity);
void player_process_keyboard_input(struct Scene *scene, uuid_t entity_id, CameraDirection camera_direction, float delta_time);
void player_process_mouse_input(struct Scene *scene, uuid_t entity_id, float xoffset, float yoffset);
void player_jump(struct Scene *scene, uuid_t entity_id);
void player_update(struct PlayerComponent *player, float delta_time);

// Inventory
void player_inventory_init(struct PlayerComponent *player, int capacity);
// bool scene_player_add_item(struct Scene *scene, uuid_t player_entity_id, int item_id, int count);
