#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <cglm/cglm.h>
//#include "glad/glad.h"

typedef enum {
    CAMERA_FORWARD = 1,
    CAMERA_BACKWARD,
    CAMERA_LEFT,
    CAMERA_RIGHT,
    CAMERA_UP,
    CAMERA_DOWN
} CameraDirection;

// Default camera values
//const vec3 POSITION = {0.0f, 0.0f, 0.0f};
//const vec3 FRONT = {0.0f, 0.0f, -1.0f};
//const vec3 UP = {0.0f, 1.0f, 0.0f};

//const float YAW         = -90.0f;
//const float PITCH       =  0.0f;
//const float FOV         =  45.0f;
//const float SENSITIVITY =  0.1f;
//const float SPEED       =  2.5f;

struct Camera {
  vec3 position;
  vec3 front;
  vec3 up;
  vec3 right;

  float yaw;
  float pitch;
  float fov;
  float sensitivity;
  float speed;
};

// Create camera with default values
Camera *camera_create(vec3 position, vec3 up, float yaw, float pitch, float fov, float sensitivity, float speed);

// Get view matrix
void camera_get_view_matrix(struct Camera *camera, mat4 view);

// Handle device input
void camera_process_keyboard_input(struct Camera *camera, CameraDirection direction, float deltaTime); 
void camera_process_mouse_input(struct Camera *camera, float xoffset, float yoffset);
void camera_process_scroll_input(struct Camera *camera, double yoffset);
void camera_update_vectors(struct Camera *camera);

#endif
