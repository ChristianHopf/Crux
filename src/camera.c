#include <GLFW/glfw3.h>
#include "camera.h"

struct Camera *camera_create(vec3 position, vec3 up, float yaw, float pitch, float fov, float sensitivity, float speed){
  struct Camera *camera = (struct Camera *)malloc(sizeof(struct Camera));
  if (!camera){
    printf("Error: failed to allocate camera\n");
    return NULL;
  }
  glm_vec3_copy(position, camera->position); // Copy vec3 to vec3
  glm_vec3_copy(up, camera->up);
  camera->yaw = yaw;
  camera->pitch = pitch;
  camera->fov = fov;
  camera->sensitivity = sensitivity;
  camera->speed = speed;
  camera_update_vectors(camera);
  return camera;
}

void camera_get_view_matrix(struct Camera *camera, mat4 view){
	glm_lookat(camera->position, (vec3){camera->position[0] + camera->front[0], camera->position[1] + camera->front[1], camera->position[2] + camera->front[2]}, camera->up, view);
}

void camera_process_keyboard_input(struct Camera *camera, CameraDirection direction, float deltaTime){
  float velocity = (float)(camera->speed * deltaTime);
	if (direction == CAMERA_FORWARD){
    vec3 forward = {camera->front[0], 0.0f, camera->front[2]};
		glm_vec3_scale(forward, velocity, forward);
		glm_vec3_add(camera->position, forward, camera->position);
	}
	if (direction == CAMERA_BACKWARD){
    vec3 backward = {camera->front[0], 0.0f, camera->front[2]};
		glm_vec3_scale(backward, velocity, backward);
		glm_vec3_sub(camera->position, backward, camera->position);
	}
	if (direction == CAMERA_LEFT){
    // I could just leave these since left and right don't affect pitch,
    // but I might want to implement leaning in the future
    vec3 left = {camera->right[0], 0.0f, camera->right[2]};
    glm_vec3_scale(left, velocity, left);
    glm_vec3_sub(camera->position, left, camera->position);
	}
	if (direction == CAMERA_RIGHT){
    vec3 right = {camera->right[0], 0.0f, camera->right[2]};
    glm_vec3_scale(right, velocity, right);
    glm_vec3_add(camera->position, right, camera->position);
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

void camera_process_mouse_input(struct Camera *camera, float xoffset, float yoffset){
  // Multiply offset by sensitivity
	xoffset *= camera->sensitivity;
	yoffset *= camera->sensitivity;

	// Add offset to yaw and pitch values
	camera->yaw += xoffset;
	camera->pitch += yoffset;

	// Disallow lookat flip by looking parallel to y axis
	if (camera->pitch > 89.0f) camera->pitch = 89.0f;
	if (camera->pitch < -89.0f) camera->pitch = -89.0f;

  // Update cameraFront and cameraRight
  camera_update_vectors(camera);
}

void camera_process_scroll_input(struct Camera *camera, double yoffset){
	camera->fov -= (float)yoffset;
	if (camera->fov <= 1.0f) camera->fov = 1.0f;
	if (camera->fov >= 45.0f) camera->fov = 45.0f;
}

void camera_update_vectors(struct Camera *camera){
  // Calculate new cameraFront vector
  vec3 direction;
	direction[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	direction[1] = sin(glm_rad(camera->pitch));
	direction[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	glm_vec3_normalize(direction);
	glm_vec3_copy(direction, camera->front);
  // Calculate cameraRight vector
  vec3 right;
  glm_vec3_cross(camera->front, camera->up, right);
  glm_vec3_normalize(right);
  glm_vec3_copy(right, camera->right);
}
