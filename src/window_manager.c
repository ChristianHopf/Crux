#include <stdio.h>
#include "window_manager.h"

static GLFWwindow *global_window = NULL;

void window_manager_init(GLFWwindow *window){
  global_window = window;
}

void window_capture_cursor(){
  if (!global_window){
    fprintf(stderr, "Error: Window not initialized in window_capture_cursor\n");
    return;
  }

  glfwSetInputMode(global_window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void window_release_cursor(){
  if (!global_window){
    fprintf(stderr, "Error: Window not initialized in window_release_cursor\n");
    return;
  }

  glfwSetInputMode(global_window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
