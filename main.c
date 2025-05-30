#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include "stb_image/stb_image.h"
#include <cglm/cglm.h>
#include "camera.h"
#include "shader.h"
#include "model.h"

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

// Screen settings
const unsigned int SCREEN_WIDTH = 800;
const unsigned int SCREEN_HEIGHT = 600;

// Timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// Mouse
bool firstMouse = true;
float lastX = 400.0f;
float lastY = 300.0f;

void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
  Camera *camera_ptr = (Camera *)glfwGetWindowUserPointer(window);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_FORWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_LEFT, deltaTime);
  }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_DOWN, deltaTime);
  }
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS){
    camera_process_keyboard_input(camera_ptr, CAMERA_UP, deltaTime);
  }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Camera *camera_ptr = (Camera *)glfwGetWindowUserPointer(window);

	if (firstMouse){
		lastX = (float)xpos;
		lastY = (float)ypos;
		firstMouse = false;
	}
	// Get offset from last frame, update last frame's mouse position
	float xoffset = (float)xpos - lastX;
	float yoffset = lastY - (float)ypos;
	lastX = (float)xpos;
	lastY = (float)ypos;

  // Update camera
  camera_process_mouse_input(camera_ptr, xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
  Camera *camera_ptr = (Camera *)glfwGetWindowUserPointer(window);
  camera_process_scroll_input(camera_ptr, yoffset);
}

int main(){
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "LearnOpenGL", NULL, NULL);
	if (window == NULL){
		printf("Failed to create GLFW window\n");
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// Capture mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
		return -1;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);

  // Camera
  Camera camera = camera_create((vec3){0.0f, 0.0f, 3.0f}, (vec3){0.0f, 1.0f, 0.0f}, -90.0f, 0.0f, 45.0f, 0.1f, 2.5f);
  glfwSetWindowUserPointer(window, &camera);

	// Shader program
	Shader shader = shader_create("shaders/shader.vs", "shaders/shader.fs");
	if (!shader.ID){
		printf("Error: failed to create shader program\n");
		glfwTerminate();
		return -1;
	}

  // Load models
  Model *model = model_create("resources/objects/pochita/scene.gltf");
  if (!model){
    printf("Error: failed to create Model\n");
  }

	// glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	// Render loop
	while (!glfwWindowShouldClose(window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// Handle input
		processInput(window);

		// Render (clear and replace with background of specified color)
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use the shader program
		shader_use(&shader);

		// Projection matrix for scroll zoom feature
		mat4 projection;
		glm_perspective(glm_rad(camera.fov), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f, projection);
		shader_set_mat4(&shader, "projection", projection);
		
		// Camera/view transformation
		mat4 view;
    camera_get_view_matrix(&camera, view);
		shader_set_mat4(&shader, "view", view);

    // Render loaded model
    mat4 model_matrix;
    glm_mat4_identity(model_matrix);
    glm_translate(model_matrix, (vec3){0.0f, 0.0f, 0.0f});
    glm_scale(model_matrix, (vec3){1.0f, 1.0f, 1.0f});
    shader_set_mat4(&shader, "model", model_matrix);
    model_draw(model, &shader);

		// Check and call events, swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
  }

	glfwTerminate();
	return 0;
}
