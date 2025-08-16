#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tinycthread/tinycthread.h"
#include "game_state_observer.h"

#define NUM_BUFFERS 4
#define BUFFER_FRAMES 8192
#define MAX_SOUND_EFFECTS 32
#define MAX_SOURCES 64

// Forward declarations
struct PlayerComponent;
struct Entity;

struct AudioStream {
  SNDFILE *file;
  SF_INFO info;
  ALenum format;
  ALuint buffers[NUM_BUFFERS];
  ALuint source;
  float *temp_buffer;

  // Threading
  thrd_t audio_thread;
  bool stop_audio;
};

struct SoundEffect {
  char *name;
  ALuint buffer;
};

struct AudioComponent {
  ALuint source_id;
  int sound_effect_index;
  bool is_playing;
  vec3 position;
};

struct AudioManager {
  // OpenAL device and context
  ALCdevice *device;
  ALCcontext *context;

  // Sources
  ALuint sources[MAX_SOURCES];
  int num_active_sources;

  // Music stream
  struct AudioStream *audio_stream;

  // Sound effects
  struct SoundEffect sound_effects[MAX_SOUND_EFFECTS];
  int num_sound_effects;

  bool paused;
};


// Init, teardown, getter (later, could accept an options object for configuration)
void audio_manager_init();
void audio_manager_free();
struct AudioManager *audio_manager_get_global();

// Add and remove sources
bool audio_add_source(ALuint source);
bool audio_remove_source(ALuint source);

// Pause and unpause
void audio_pause();
void audio_unpause();

// Streaming functions
void audio_stream_create(char *path);
void audio_stream_destroy(struct AudioStream *stream);
int audio_stream_update(void *arg);
bool fill_buffer(struct AudioStream *stream, ALuint buffer);

// One shot sound functions
void audio_sound_effect_create(char *path, char *name);
void audio_sound_effect_play(struct SoundEffect *sound_effect);

// AudioComponent
struct AudioComponent *audio_component_create(struct Entity *entity, int sound_effect_index);
void audio_component_destroy(struct AudioComponent *audio_component);
void audio_component_play(struct AudioComponent *audio_component);

// Listener
void audio_listener_update(struct PlayerComponent *player);

// Observing game state
struct GameStateObserver *audio_game_state_observer_create();
void audio_game_state_changed(void *instance, GameState *game_state);
