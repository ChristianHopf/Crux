#include <cglm/vec3.h>
#include <uuid/uuid.h>
#include "player.h"
#include "physics/world.h"

// struct Player *player_create(struct Model *model, Shader *shader, vec3 position, vec3 rotation, vec3 scale, vec3 velocity, vec3 camera_offset, float camera_height, bool render_entity, int inventory_capacity){
//
//   // Allocate player
//   struct Player *player = (struct Player *)calloc(1, sizeof(struct Player));
//   if (!player){
//     fprintf(stderr, "Error: failed to allocate player in player_create\n");
//     return NULL;
//   }
//
//   // Init camera
//   vec3 cameraPos = {0.0f, 0.0f, 0.0f};
//   vec3 cameraUp = {0.0f, 1.0f, 0.0f};
//   float yaw = -90.0f;
//   float pitch = 0.0f;
//   float fov = 90.0f;
//   float sensitivity = 0.1f;
//   float speed = 2.5f;
//   struct Camera *camera = camera_create(cameraPos, cameraUp, yaw, pitch, fov, sensitivity, speed);
//   if (!camera){
//     printf("Error: failed to create camera in player init\n");
//     return NULL;
//   }
//   player->camera = camera;
//
//   // Add entity information (model, shader, etc)
//   player->entity = (struct Entity *)calloc(1, sizeof(struct Entity));
//   if (!player->entity){
//     fprintf(stderr, "Error: failed to allocate entity in player_init\n");
//     return NULL;
//   }
//   uuid_generate(player->entity->id);
//   player->entity->model = model;
//   player->entity->shader = shader;
//   glm_vec3_copy(position, player->entity->position);
//   glm_vec3_copy(rotation, player->entity->rotation);
//   glm_vec3_copy(scale, player->entity->scale);
//   glm_vec3_copy(velocity, player->entity->velocity);
//   glm_vec3_copy(camera_offset, player->camera_offset);
//   glm_vec3_copy(camera_offset, player->rotated_offset);
//   player->camera_height = camera_height;
//   player->render_entity = render_entity;
//
//   // AudioComponent
//   player->entity->audio_component = audio_component_create(player->entity, 0);
//
//   // Set listener position to camera position
//   audio_listener_update(player);
//
//   // Initialize inventory
//   player_inventory_init(player, inventory_capacity);
//
//   return player;
// }

void player_process_keyboard_input(struct PlayerComponent *player, CameraDirection direction, float delta_time){
  float velocity = (float)(player->camera->speed * delta_time);
	if (direction == CAMERA_FORWARD){
    vec3 forward = {player->camera->front[0], 0.0f, player->camera->front[2]};
    glm_vec3_normalize(forward);
		glm_vec3_scale(forward, velocity, forward);
		glm_vec3_add(player->entity->physics_body->position, forward, player->entity->physics_body->position);
	}
	if (direction == CAMERA_BACKWARD){
    vec3 backward = {player->camera->front[0], 0.0f, player->camera->front[2]};
    glm_vec3_normalize(backward);
		glm_vec3_scale(backward, velocity, backward);
		glm_vec3_sub(player->entity->physics_body->position, backward, player->entity->physics_body->position);
	}
	if (direction == CAMERA_LEFT){
    // I could just leave these since left and right don't affect pitch,
    // but I might want to implement leaning in the future
    vec3 left = {player->camera->right[0], 0.0f, player->camera->right[2]};
    glm_vec3_scale(left, velocity, left);
    glm_vec3_sub(player->entity->physics_body->position, left, player->entity->physics_body->position);
	}
	if (direction == CAMERA_RIGHT){
    vec3 right = {player->camera->right[0], 0.0f, player->camera->right[2]};
    glm_vec3_scale(right, velocity, right);
    glm_vec3_add(player->entity->physics_body->position, right, player->entity->physics_body->position);
	}
	// if (direction == CAMERA_DOWN){
	// 	vec3 down;
	// 	glm_vec3_copy(camera->up, down);
	// 	glm_vec3_scale(down, velocity, down);
	// 	glm_vec3_sub(camera->position, down, camera->position);
	// }
	// if (direction == CAMERA_UP){
	// 	vec3 up;
	// 	glm_vec3_copy(camera->up, up);
	// 	glm_vec3_scale(up, velocity, up);
	// 	glm_vec3_add(camera->position, up, camera->position);
	//}
}

void player_process_mouse_input(struct PlayerComponent *player, float xoffset, float yoffset){
  struct Camera *camera = player->camera;

  // Multiply offset by sensitivity
	xoffset *= camera->sensitivity;
	yoffset *= camera->sensitivity;

	// Add offset to yaw and pitch values
	camera->yaw += xoffset;
	camera->pitch += yoffset;

	// Disallow lookat flip by looking parallel to y axis
	if (camera->pitch > 89.0f) camera->pitch = 89.0f;
	if (camera->pitch < -89.0f) camera->pitch = -89.0f;

  // Keep yaw between 0 and 360 degrees
  if (camera->yaw >= 360.0f) camera->yaw -= 360.0f;
  if (camera->yaw < 0.0f) camera->yaw += 360.0f;

  // Update camera direction using offset
  float r = glm_vec3_norm(player->camera_offset);
  vec3 direction;
  if (r > 0){
    vec3 update;
    update[0] = -cosf(glm_rad(camera->pitch)) * cosf(glm_rad(camera->yaw));
    update[1] = -sinf(glm_rad(camera->pitch));
    update[2] = -cosf(glm_rad(camera->pitch)) * sinf(glm_rad(camera->yaw));
    update[0] = clamp(update[0], -r, r);
    update[1] = clamp(update[1], -r, r);
    update[2] = clamp(update[2], -r, r);

    // Rotate camera offset by yaw
    mat4 rotation;
    glm_mat4_identity(rotation);

    glm_rotate_y(rotation, glm_rad(-camera->yaw - 90.0f), rotation);
    glm_rotate_x(rotation, glm_rad(camera->pitch), rotation);
    glm_mat4_mulv3(rotation, player->camera_offset, 0.0f, player->rotated_offset);

    glm_vec3_add(player->entity->position, update, player->camera->position);
    glm_vec3_add(player->camera->position, player->rotated_offset, player->camera->position);

    // Translate camera position based on pitch after rotating based on yaw
    // vec3 pitch_translation;
    // pitch_translation[0] = -cos(glm_rad(camera->yaw)) * cos(glm_rad(-camera->pitch));
    // pitch_translation[1] = sin(glm_rad(-camera->pitch));
    // pitch_translation[2] = -cos(glm_rad(camera->pitch)) * sin(glm_rad(camera->yaw));

    // Calculate new cameraFront vector
    vec3 target;
    glm_vec3_add(player->entity->position, player->rotated_offset, target);
    glm_vec3_sub(target, camera->position, direction);
  }
  else{
    direction[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
    direction[1] = sin(glm_rad(camera->pitch));
    direction[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
  }

  // Calculate new cameraFront vector 
  glm_vec3_normalize(direction);
  glm_vec3_copy(direction, camera->front);
  // Calculate cameraRight vector
  vec3 right;
  glm_vec3_cross(camera->front, camera->up, right);
  glm_vec3_normalize(right);
  glm_vec3_copy(right, camera->right);

  // Rotate PhysicsBody
  // (This assumes the model for the player will always have been created facing
  // the positive z direction, and thus its rotation about the y axis must be adjusted
  // to face the same direction as the camera. I may also want to make an API for setting
  // physics body values instead of directly mutating them.)
  player->entity->physics_body->rotation[0] = 0.0f;
  player->entity->physics_body->rotation[1] = -camera->yaw + 90.0f;
  player->entity->physics_body->rotation[2] = 0.0f;
}

void player_jump(struct PlayerComponent *player){
  // Reset at_rest
  struct PhysicsBody *body = player->entity->physics_body;
  body->at_rest = false;

  // Apply an impulse to player->physics_body->velocity
  body->velocity[1] = 3.0f;
}

void player_update(struct PlayerComponent *player, float delta_time){
  glm_vec3_copy(player->entity->physics_body->position, player->entity->position);
  glm_vec3_copy(player->entity->physics_body->rotation, player->entity->rotation);
  glm_vec3_copy(player->entity->physics_body->velocity, player->entity->velocity);
  // Add Camera offset
  glm_vec3_add(player->entity->position, player->rotated_offset, player->camera->position);
  player->camera->position[1] += player->camera_height;

  // Update audio source position
  alSource3f(player->entity->audio_component->source_id, AL_POSITION, player->entity->position[0], player->entity->position[1], player->entity->position[2]);
  ALenum position_error = alGetError();
  if (position_error != AL_NO_ERROR){
    fprintf(stderr, "Error matching Entity audio_source position with entity position in scene_update: %d\n", position_error);
  }

  // Update listener position and orientation
  audio_listener_update(player);
}

void player_inventory_init(struct PlayerComponent *player, int capacity){
  player->inventory.items = (struct ItemComponent *)calloc(capacity, sizeof(struct ItemComponent));
  if (!player->inventory.items){
    fprintf(stderr, "Error: failed to allocate Items in player_inventory_init\n");
    return;
  }
  player->inventory.size = 0;
  player->inventory.capacity = capacity;
}

bool player_add_item(struct PlayerComponent *player, int item_id, int count){
  if (player->inventory.size >= player->inventory.capacity){
    return false;
  }
  return true;
}
