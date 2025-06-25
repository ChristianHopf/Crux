#include <cglm/cam.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include "tinycthread/tinycthread.h"
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <stdbool.h>
#include "stb_image/stb_image.h"
#include <cglm/cglm.h>
#include "camera.h"
#include "shader.h"
#include "model.h"
#include "scene.h"
#include "player.h"
#include "text.h"
#include "audio_manager.h"

typedef struct {
  GLFWwindow *window;
  struct Scene *active_scene;
  // Timing
  float deltaTime;
  float lastFrame;
  // Multithreading
  mtx_t scene_mutex;
  cnd_t render_signal;
  cnd_t render_done_signal;
  // pthread_mutex_t scene_mutex;
  // pthread_cond_t render_signal;
  // pthread_cond_t render_done_signal;
  bool render_ready;
} Engine;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_callback(GLFWwindow *window, double xpos, double ypos);
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

// Screen settings
const unsigned int SCREEN_WIDTH = 1280;
const unsigned int SCREEN_HEIGHT = 720;

// Mouse
bool firstMouse = true;
float lastX = 512.0f;
float lastY = 384.0f;

// Lighting
vec3 lightPos = {1.2f, 0.5f, 2.0f};

static int last_space_state = GLFW_RELEASE;
void processInput(GLFWwindow *window){
	if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS){
		glfwSetWindowShouldClose(window, 1);
	}
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;

  // Don't process input (other than the Escape key) if the game is paused
  if (engine->active_scene->paused){
    return;
  }

  // Camera movement
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_FORWARD, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_BACKWARD, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_LEFT, engine->deltaTime);
  }
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_RIGHT, engine->deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS){
    camera_process_keyboard_input(camera, CAMERA_DOWN, engine->deltaTime);
  }

  // Only process these inputs a single time per press
  int space_state = glfwGetKey(window, GLFW_KEY_SPACE);
	if (space_state == GLFW_PRESS && last_space_state == GLFW_RELEASE){
    player_jump(&engine->active_scene->player);
    //camera_process_keyboard_input(camera, CAMERA_UP, engine->deltaTime);
  }
  last_space_state = space_state;
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height){
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow *window, double xpos, double ypos){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);

  // Don't process input (other than the Escape key) if the game is paused
  if (engine->active_scene->paused){
    // A better way to handle this: on pause, set firstMouse to true.
    // Would have to move it from a main.c global var
    //firstMouse = true;
    return;
  }

  struct Camera *camera = engine->active_scene->player.camera;

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
  camera_process_mouse_input(camera, xoffset, yoffset);
}

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset){
  Engine *engine = (Engine *)glfwGetWindowUserPointer(window);
  struct Camera *camera = engine->active_scene->player.camera;
  camera_process_scroll_input(camera, yoffset);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods){
  struct Scene *active_scene = ((Engine *)glfwGetWindowUserPointer(window))->active_scene;

  // Pause
  if (key == GLFW_KEY_P && action == GLFW_PRESS){
    scene_pause(active_scene);
  }
}

Engine *engine_create(){
  Engine *engine = (Engine *)calloc(1, sizeof(Engine));
  if (!engine){
    printf("Error: failed to allocate Engine\n");
    return NULL;
  }

	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  // Create window
	GLFWwindow *window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Crux", NULL, NULL);
	if (window == NULL){
		printf("Failed to create GLFW window\n");
    free(engine);
		glfwTerminate();
		return NULL;
	}
  engine->window = window;
  glfwMakeContextCurrent(engine->window);
	glfwSetFramebufferSizeCallback(engine->window, framebuffer_size_callback);
	glfwSetCursorPosCallback(engine->window, mouse_callback);
	glfwSetScrollCallback(engine->window, scroll_callback);
  glfwSetKeyCallback(engine->window, key_callback);
  glfwSetWindowUserPointer(engine->window, engine);

	// Capture mouse
	glfwSetInputMode(engine->window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// Init GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
		printf("Failed to initialize GLAD\n");
    free(engine);
		return NULL;
	}

  // Flip textures across y-axis
  stbi_set_flip_vertically_on_load(true);

	// Configure global OpenGL state
	glEnable(GL_DEPTH_TEST);
  glEnable(GL_CULL_FACE);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  //glBlendFunc(GL_ONE, GL_ONE); // additive blending

  //glDepthFunc(GL_ALWAYS);
  //glEnable(GL_STENCIL_TEST);
  //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // engine->active_scene = scene_create(true);
  engine->active_scene = scene_init("scenes/bouncehouse2.json");
  if (!engine->active_scene){
    printf("Error: failed to create scene\n");
    free(engine);
    glfwTerminate();
    return NULL;
  }

  // Timing
  engine->deltaTime = 0.0f;
  engine->lastFrame = 0.0f;

  // Scene mutex
  mtx_init(&engine->scene_mutex, mtx_plain);
  cnd_init(&engine->render_signal);
  cnd_init(&engine->render_done_signal);
  // pthread_mutex_init(&engine->scene_mutex, NULL);
  // pthread_cond_init(&engine->render_signal, NULL);
  // pthread_cond_init(&engine->render_done_signal, NULL);
  engine->render_ready = false;

  return engine;
}

// Just slap everything down outside main and get the thread working
ALCdevice *device;
ALCcontext *context;
ALuint buffers[4];
ALuint source;

volatile int stop_audio = 0;


// SEPARATE THREADS
// void *process_audio(void *arg){
//   alcMakeContextCurrent(context);
//   while (!stop_audio){
//     ALint state, processed, queued;
//     alGetSourcei(source, AL_SOURCE_STATE, &state);
//     alGetSourcei(source, AL_BUFFERS_PROCESSED, &processed);
//     alGetSourcei(source, AL_BUFFERS_QUEUED, &queued);
//     printf("Processed %d buffers\n", processed);
//     if (processed > 0){
//       printf("Processed buffers: %d, queued buffers: %d\n", processed, queued);
//       for(int i = 0; i < processed; i++){
//         ALuint buf;
//         alSourceUnqueueBuffers(source, 1, &buf);
//         alSourceQueueBuffers(source, 1, &buf);
//       }
//     }
//     if (state == AL_PLAYING){
//       // printf("AUDIO PLAYING\n");
//       alcProcessContext(context);
//     }
//     usleep(1000);
//   }
//   alcMakeContextCurrent(NULL);
//   return NULL;
// }

void *render_thread(void *arg){
  Engine *engine = (Engine *)arg;
  glfwMakeContextCurrent(engine->window);
  while (!glfwWindowShouldClose(engine->window)){
    // Lock mutex
    mtx_lock(&engine->scene_mutex);
    // pthread_mutex_lock(&engine->scene_mutex);

    // Wait for render signal
    while (!engine->render_ready){
      cnd_wait(&engine->render_signal, &engine->scene_mutex);
      // pthread_cond_wait(&engine->render_signal, &engine->scene_mutex);
    }

    // Render scene
    scene_render(engine->active_scene);
    glfwSwapBuffers(engine->window);

    // Set render_ready back to false, signal rendering is done, unlock mutex
    engine->render_ready = false;
    cnd_signal(&engine->render_done_signal);
    mtx_unlock(&engine->scene_mutex);
    // pthread_cond_signal(&engine->render_done_signal);
    // pthread_mutex_unlock(&engine->scene_mutex);
  }

  glfwMakeContextCurrent(NULL);
  return NULL;
}

int main(){
  Engine *engine = engine_create();
  if (!engine){
    printf("Error: failed to create Engine\n");
    glfwTerminate();
    return -1;
  }

  // Init OpenAL device and context
  // ALCdevice *device;
  // ALCcontext *context;

  device = alcOpenDevice(NULL);
  if (!device){
    ALCenum error = alcGetError(NULL);
    fprintf(stderr, "Error: failed to open OpenAL device: %d\\n", error);
    glfwTerminate();
    free(engine);
    return 0;
  }
  context = alcCreateContext(device, NULL);
  if (!context){
    fprintf(stderr, "Error: failed to create OpenAL context\n");
    alcCloseDevice(device);
    glfwTerminate();
    free(engine);
    return 0;
  }
  alcMakeContextCurrent(context);

  // // Load WAV with sndfile
  // SF_INFO info;
  // SNDFILE *mp3_file = sf_open("resources/music/mookid.wav", SFM_READ, &info);
  // if (!mp3_file){
  //   fprintf(stderr, "Error: failed to open %s: %s\n", "resources/music/mookid.wav", sf_strerror(NULL));
  //   alcMakeContextCurrent(NULL);
  //   alcDestroyContext(context);
  //   alcCloseDevice(device);
  //   glfwTerminate();
  //   free(engine);
  //   return 0;
  // }
  //
  // float *mp3_data = (float *)malloc(info.frames * info.channels * sizeof(float));
  // sf_count_t read_frames = sf_readf_float(mp3_file, mp3_data, info.frames);
  // // sf_close(mp3_file);
  // if (read_frames != info.frames){
  //   fprintf(stderr, "Error: failed to read correct number of frames into wav_data\n");
  //   free(mp3_data);
  //   alcMakeContextCurrent(NULL);
  //   alcDestroyContext(context);
  //   alcCloseDevice(device);
  //   glfwTerminate();
  //   free(engine);
  //   return 0;
  // }
  //
  // ALenum format;
  // if (info.channels == 1){
  //   format = AL_FORMAT_MONO_FLOAT32;
  // }
  // else{
  //   format = AL_FORMAT_STEREO_FLOAT32;
  // }
  //
  // // Buffer audio and create a source
  // // ALuint buffer, source;
  // alGenBuffers(4, buffers);
  // alGetError();
  // sf_count_t frames_per_buffer = 4096;
  // // sf_count_t frames_per_buffer = info.frames / 4;
  // printf("Audio buffer size: %ld frames, %ld samples per buffer\n", 
  //      info.frames, frames_per_buffer);
  // for(int i = 0; i < 4; i++){
  //   sf_count_t offset = i * frames_per_buffer * info.channels;
  //   sf_count_t size;
  //   if (i == 3){
  //     size = (info.frames - offset / info.channels) * info.channels * sizeof(float);
  //   }
  //   else{
  //     size = frames_per_buffer * info.channels * sizeof(float);
  //   }
  //   alBufferData(buffers[i], format, mp3_data + offset, size, info.samplerate);
  //   ALenum buffer_error = alGetError();
  //   if (buffer_error != AL_NO_ERROR){
  //     fprintf(stderr, "Error: alGetError returned error %d for alBufferData with buffer %d\n", buffer_error, i);
  //   }
  // }

  // alBufferData(buffer, format, mp3_data, info.frames * info.channels * sizeof(float), info.samplerate);
  // free(mp3_data);
  //
  // alGenSources(1, &source);
  // alSourceQueueBuffers(source, 4, buffers);
  // if (alGetError() != AL_NO_ERROR){
  //   fprintf(stderr, "Error: alGetError returned error %d in alSourceQueueBuffers\n", alGetError());
  // }
  // alSourcef(source, AL_GAIN, 1.0f);
  // alSourcef(source, AL_PITCH, 1.0f);
  // // alSourcei(source, AL_LOOPING, AL_TRUE);
  //
  // alSourcePlay(source);

  // Load font
  load_font_face();

  // Release context for render thread to use it later
  glfwMakeContextCurrent(NULL);

  struct AudioStream *audio_stream = audio_stream_create("resources/music/mookid.wav");

  thrd_t render_thrd;
  thrd_create(&render_thrd, render_thread, engine);

	// Render loop
	while (!glfwWindowShouldClose(engine->window)){
		// Per-frame timing logic
		float currentFrame = (float)(glfwGetTime());

    // Lock scene mutex, do timing and updating
    mtx_lock(&engine->scene_mutex);
		engine->deltaTime = currentFrame - engine->lastFrame;
		engine->lastFrame = currentFrame;

		printf("FPS: %f\n", 1.0 / engine->deltaTime);

		// Handle input, update
		processInput(engine->window);

    float update_start_time = glfwGetTime();
		scene_update(engine->active_scene, engine->deltaTime);
    float update_end_time = glfwGetTime();
    printf("scene update took %.2f ms\n", (update_end_time - update_start_time) * 1000.0);

    // Set render_ready to true, signal render thread to work,
    float render_start_time = glfwGetTime();
    engine->render_ready = true;
    cnd_signal(&engine->render_signal);
    while(engine->render_ready){
      cnd_wait(&engine->render_done_signal, &engine->scene_mutex);
    }
    mtx_unlock(&engine->scene_mutex);

    float render_end_time = glfwGetTime();
    printf("Render took %.2f ms\n", (render_end_time - render_start_time) * 1000.0);

		// scene_render(engine->active_scene);

		// Check and call events, swap buffers
		// glfwSwapBuffers(engine->window);
		glfwPollEvents();
  }

  audio_stream_destroy(audio_stream);
  thrd_join(render_thrd, NULL);
  mtx_destroy(&engine->scene_mutex);
  cnd_destroy(&engine->render_signal);
  cnd_destroy(&engine->render_done_signal);
  alcMakeContextCurrent(NULL);
  alcDestroyContext(context);
  alcCloseDevice(device);

  // sf_close(mp3_file);

  glfwDestroyWindow(engine->window);
	glfwTerminate();
  free(engine);
	return 0;
}
