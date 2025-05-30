#include "camera.h"
#include <GLFW/glfw3.h>

Camera camera_create(vec3 position, vec3 up, float yaw, float pitch, float fov, float sensitivity, float speed){
  Camera camera;
  glm_vec3_copy(position, camera.position); // Copy vec3 to vec3
  glm_vec3_copy(up, camera.up);
  camera.yaw = yaw;
  camera.pitch = pitch;
  camera.fov = fov;
  camera.sensitivity = sensitivity;
  camera.speed = speed;
  camera_update_vectors(&camera);
  return camera;
}

void camera_get_view_matrix(Camera *camera, mat4 view){
	glm_lookat(camera->position, (vec3){camera->position[0] + camera->front[0], camera->position[1] + camera->front[1], camera->position[2] + camera->front[2]}, camera->up, view);
}

void camera_process_keyboard_input(Camera *camera, CameraDirection direction, float deltaTime){
  float velocity = (float)(camera->speed * deltaTime);
	if (direction == CAMERA_FORWARD){
		vec3 forward;
		glm_vec3_scale(camera->front, velocity, forward);
		glm_vec3_add(camera->position, forward, camera->position);
	}
	if (direction == CAMERA_BACKWARD){
		vec3 backward;
		glm_vec3_scale(camera->front, velocity, backward);
		glm_vec3_sub(camera->position, backward, camera->position);
	}
	if (direction == CAMERA_LEFT){
    vec3 left;
    glm_vec3_scale(camera->right, velocity, left);
    glm_vec3_sub(camera->position, left, camera->position);
	}
	if (direction == CAMERA_RIGHT){
    vec3 right;
    glm_vec3_scale(camera->right, velocity, right);
    glm_vec3_add(camera->position, right, camera->position);
	}
	if (direction == CAMERA_DOWN){
		vec3 down;
		glm_vec3_copy(camera->up, down);
		glm_vec3_scale(down, velocity, down);
		glm_vec3_sub(camera->position, down, camera->position);
	}
	if (direction == CAMERA_UP){
		vec3 up;
		glm_vec3_copy(camera->up, up);
		glm_vec3_scale(up, velocity, up);
		glm_vec3_add(camera->position, up, camera->position);
	}
}

void camera_process_mouse_input(Camera *camera, float xoffset, float yoffset){
  // Multiply offset by sensitivity
	xoffset *= camera->sensitivity;
	yoffset *= camera->sensitivity;

	// Add offset to yaw and pitch values
	camera->yaw += xoffset;
	camera->pitch += yoffset;

	// Disallow lookat flip by looking parallel to y axis
	if (camera->pitch > 89.0f) camera->pitch = 89.0f;
	if (camera->pitch < -89.0f) camera->pitch = -89.0f;

	// Calculate direction vector
	//vec3 direction;
	//direction[0] = cos(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	//direction[1] = sin(glm_rad(camera->pitch));
	//direction[2] = sin(glm_rad(camera->yaw)) * cos(glm_rad(camera->pitch));
	//glm_vec3_normalize(direction);
	//glm_vec3_copy(direction, camera->front);

  // Update cameraFront and cameraRight
  camera_update_vectors(camera);
}

void camera_process_scroll_input(Camera *camera, double yoffset){
	camera->fov -= (float)yoffset;
	if (camera->fov <= 1.0f) camera->fov = 1.0f;
	if (camera->fov >= 45.0f) camera->fov = 45.0f;
}

void camera_update_vectors(Camera *camera){
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
