#include <cglm/vec3.h>
#include "player.h"

void player_init(struct Player *player){
  vec3 cameraPos = {0.0f, 0.0f, 3.0f};
  vec3 cameraUp = {0.0f, 1.0f, 0.0f};
  float yaw = -90.0f;
  float pitch = 0.0f;
  float fov = 45.0f;
  float sensitivity = 0.1f;
  float speed = 2.5f;

  struct Camera *camera = camera_create(cameraPos, cameraUp, yaw, pitch, fov, sensitivity, speed);
  if (!camera){
    printf("Error: failed to create camera in player init\n");
    return;
  }

  player->camera = camera;
}

void player_jump(struct Player *player){
  // Basic jump: add a constant value to the player's y axis velocity
  glm_vec3_add(player->velocity, (vec3){0.0f, 3.0f, 0.0f}, player->velocity);
  printf("Player upward velocity: %f\n", player->velocity[1]);
}

void player_update(struct Player *player, float delta_time){
  // Update the camera position according to the player's velocity and delta_time
  vec3 update;
  glm_vec3_copy(player->velocity, update);
  glm_vec3_scale(update, delta_time, update);
  glm_vec3_add(update, player->camera->position, player->camera->position);
}
