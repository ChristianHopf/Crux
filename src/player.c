#include <cglm/vec3.h>
#include "player.h"

void player_init(struct Player *player){

  // Init camera
  vec3 cameraPos = {0.0f, 1.0f, 3.0f};
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
  glm_vec3_copy((vec3){0.0f, 0.0f, 0.0f}, player->velocity);
  player->is_grounded = true;

  // Set listener position to camera position
  audio_listener_update(player);
}

void player_jump(struct Player *player){
  // Basic jump: add a constant value to the player's y axis velocity
  player->velocity[1] = 3.0f;
  player->is_grounded = false;
}

void player_update(struct Player *player, float delta_time){
  if (!player->is_grounded){
    // Apply gravity to the player's velocity
    float gravity = 9.8f;
    player->velocity[1] -= gravity * delta_time;

    // Add player's velocity to the camera position, scaled by delta_time
    vec3 update;
    glm_vec3_copy(player->velocity, update);
    glm_vec3_scale(update, delta_time, update);
    glm_vec3_add(update, player->camera->position, player->camera->position);

  }
  
  // If this update makes the player hit the ground, set their velocity back to 0
  if (player->camera->position[1] <= 1.0f){
    player->camera->position[1] = 1.0f;
    player->velocity[1] = 0.0f;
    player->is_grounded = true;
  }

  // Update listener position and orientation
  audio_listener_update(player);
}


