#include <cglm/vec3.h>
#include <uuid/uuid.h>
#include "utils.h"
#include "scene.h"
#include "player.h"
#include "entity.h"
#include "camera.h"
#include "audio_manager.h"
#include "physics/world.h"

void player_process_keyboard_input(struct Scene *scene, uuid_t entity_id, CameraDirection direction, float delta_time){
  struct PlayerComponent *player = scene_get_player_by_entity_id(scene, entity_id);
  if (!player){
    fprintf(stderr, "Error: failed to get PlayerComponent in player_process_keyboard_input\n");
    return;
  }

  // TODO Some kind of physics API so that I can remove the direct reference to the PhysicsBody from Entity
  struct Entity *player_entity = scene_get_entity_by_entity_id(scene, entity_id);
  struct CameraComponent *camera_component = scene_get_camera_by_entity_id(scene, entity_id);

  float velocity = (float)(camera_component->speed * delta_time);
	if (direction == CAMERA_FORWARD){
    vec3 forward = {camera_component->front[0], 0.0f, camera_component->front[2]};
    glm_vec3_normalize(forward);
		glm_vec3_scale(forward, velocity, forward);
		glm_vec3_add(player_entity->physics_body->position, forward, player_entity->physics_body->position);
	}
	if (direction == CAMERA_BACKWARD){
    vec3 backward = {camera_component->front[0], 0.0f, camera_component->front[2]};
    glm_vec3_normalize(backward);
		glm_vec3_scale(backward, velocity, backward);
		glm_vec3_sub(player_entity->physics_body->position, backward, player_entity->physics_body->position);
	}
	if (direction == CAMERA_LEFT){
    // I could just leave these since left and right don't affect pitch,
    // but I might want to implement leaning in the future
    vec3 left = {camera_component->right[0], 0.0f, camera_component->right[2]};
    glm_vec3_scale(left, velocity, left);
    glm_vec3_sub(player_entity->physics_body->position, left, player_entity->physics_body->position);
	}
	if (direction == CAMERA_RIGHT){
    vec3 right = {camera_component->right[0], 0.0f, camera_component->right[2]};
    glm_vec3_scale(right, velocity, right);
    glm_vec3_add(player_entity->physics_body->position, right, player_entity->physics_body->position);
	}
  
  // Update transform
  struct SceneNode *player_node;
  int child_index, final_child_index;
  scene_get_node_by_entity_id(scene->root_node, player->entity_id, &child_index, &final_child_index, &player_node);
  scene_node_update(scene, player_node);
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

void player_process_mouse_input(struct Scene *scene, uuid_t entity_id, float xoffset, float yoffset){
  struct PlayerComponent *player = scene_get_player_by_entity_id(scene, entity_id);
  if (!player){
    fprintf(stderr, "Error: failed to get PlayerComponent in player_process_mouse_input\n");
    return;
  }
  struct CameraComponent *camera = scene_get_camera_by_entity_id(scene, entity_id);
  if (!camera){
    fprintf(stderr, "Error: failed to get CameraComponent in player_process_mouse_input\n");
    return;
  }

  struct Entity *player_entity = scene_get_entity_by_entity_id(scene, entity_id);
  if (!player_entity){
    fprintf(stderr, "Error: failed to get player Entity in player_process_mouse_input\n");
    return;
  }

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

    glm_vec3_add(player_entity->position, update, camera->position);
    glm_vec3_add(camera->position, player->rotated_offset, camera->position);

    // Translate camera position based on pitch after rotating based on yaw
    // vec3 pitch_translation;
    // pitch_translation[0] = -cos(glm_rad(camera->yaw)) * cos(glm_rad(-camera->pitch));
    // pitch_translation[1] = sin(glm_rad(-camera->pitch));
    // pitch_translation[2] = -cos(glm_rad(camera->pitch)) * sin(glm_rad(camera->yaw));

    // Calculate new cameraFront vector
    vec3 target;
    glm_vec3_add(player_entity->position, player->rotated_offset, target);
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
  player_entity->physics_body->rotation[0] = 0.0f;
  player_entity->physics_body->rotation[1] = -camera->yaw + 90.0f;
  player_entity->physics_body->rotation[2] = 0.0f;
}

void player_jump(struct Scene *scene, uuid_t entity_id){
  struct Entity *player_entity = scene_get_entity_by_entity_id(scene, entity_id);
  if (!player_entity){
    fprintf(stderr, "Error: failed to get player Entity in player_jump\n");
    return;
  }

  // Reset at_rest
  struct PhysicsBody *body = player_entity->physics_body;
  body->at_rest = false;

  // Apply an impulse to player->physics_body->velocity
  body->velocity[1] = 3.0f;
}

void player_update(struct Scene *scene, uuid_t entity_id, float delta_time){
  // Possibly remove this if in the future audio_listener_update is refactored
  // to not take a pointer to a PlayerComponent
  struct PlayerComponent *player_component = scene_get_player_by_entity_id(scene, entity_id);
  if (!player_component){
    fprintf(stderr, "Error: failed to get PlayerComponent in player_update\n");
    return;
  }
  struct Entity *player_entity = scene_get_entity_by_entity_id(scene, entity_id);
  if (!player_entity){
    fprintf(stderr, "Error: failed to get player Entity in player_update\n");
  }
  struct CameraComponent *camera_component = scene_get_camera_by_entity_id(scene, entity_id);
  if (!camera_component){
    fprintf(stderr, "Error: failed to get CameraComponent in player_update\n");
  }
  struct AudioComponent *audio_component = scene_get_audio_component_by_entity_id(scene, entity_id);

  glm_vec3_copy(player_entity->physics_body->position, player_entity->position);
  glm_vec3_copy(player_entity->physics_body->rotation, player_entity->rotation);
  glm_vec3_copy(player_entity->physics_body->velocity, player_entity->velocity);
  // Add Camera offset
  glm_vec3_add(player_entity->position, player_component->rotated_offset, camera_component->position);
  camera_component->position[1] += player_component->camera_height;

  // Update audio source position
  alSource3f(audio_component->source_id, AL_POSITION, player_entity->position[0], player_entity->position[1], player_entity->position[2]);
  ALenum position_error = alGetError();
  if (position_error != AL_NO_ERROR){
    fprintf(stderr, "Error matching Entity audio_source position with entity position in scene_update: %d\n", position_error);
  }

  // Update listener position and orientation
  audio_listener_update(scene, entity_id);
}
