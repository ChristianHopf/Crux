#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <cglm/cglm.h>
#include <stdlib.h>
#include <stdbool.h>
#include <uuid/uuid.h>
#include "tinycthread/tinycthread.h"
#include "game_state_observer.h"

#define NUM_BUFFERS 4
#define BUFFER_FRAMES 8192
#define MAX_SOUND_EFFECTS 32
#define MAX_SOURCES 64
#define MAX_COMPONENT_SOURCES 32

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
  uuid_t entity_id;
  ALuint source_id[MAX_COMPONENT_SOURCES];
  unsigned int num_active_sources;
  // bool is_playing;
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
bool audio_manager_init(struct AudioManager *audio_manager);
void audio_manager_destroy(struct AudioManager *audio_manager);
struct AudioManager *audio_manager_get_global();

// Add and remove sources
bool audio_add_source(struct AudioManager *audio_manager, ALuint source);
bool audio_remove_source(struct AudioManager *audio_manager, ALuint source);

// Pause and unpause
void audio_pause(struct AudioManager *audio_manager);
void audio_unpause(struct AudioManager *audio_manager);

// Streaming functions
void audio_stream_create(struct AudioManager *audio_manager, char *path);
void audio_stream_destroy(struct AudioStream *stream);
int audio_stream_update(void *arg);
bool fill_buffer(struct AudioStream *stream, ALuint buffer);

// One shot sound functions
void audio_sound_effect_create(struct AudioManager *audio_manager, char *path, char *name);
void audio_sound_effect_play(struct SoundEffect *sound_effect);

// AudioComponent
void audio_component_create(struct Scene *scene, uuid_t entity_id, struct AudioManager *audio_manager, int sound_effect_index);
void audio_component_destroy(struct AudioManager *audio_manager, struct AudioComponent *audio_component);
void audio_component_play(struct AudioManager *audio_manager, struct AudioComponent *audio_component, int sound_effect_index);

// Listener
void audio_listener_update(struct Scene *scene, uuid_t entity_id);

// Observing game state
struct GameStateObserver *audio_game_state_observer_create(struct AudioManager *audio_manager);
void audio_game_state_changed(void *instance, GameState *game_state);
