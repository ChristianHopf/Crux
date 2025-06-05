#include "player.h"

void player_init(struct Player *player, struct Camera *camera){
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
    return NULL;
  }

  player->camera = camera;
}
