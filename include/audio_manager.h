#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tinycthread/tinycthread.h"

#define NUM_BUFFERS 4
#define BUFFER_FRAMES 8192

#define MAX_SOUND_EFFECTS 128

struct AudioStream {
  SNDFILE *file;
  SF_INFO info;
  ALenum format;
  ALuint buffers[NUM_BUFFERS];
  ALuint source;
  float *temp_buffer;

  thrd_t audio_thread;

  bool stop_audio;
};

struct SoundEffect {
  char *name;
  ALuint buffer;
};

// Extern vars
extern ALCdevice *audio_device;
extern ALCcontext *audio_context;
extern struct SoundEffect sound_effects[MAX_SOUND_EFFECTS];
extern int num_sound_effects;

struct AudioStream *audio_stream_create(char *path);
void audio_stream_destroy(struct AudioStream *stream);
int audio_stream_update(void *arg);

// HELPERS
bool fill_buffer(struct AudioStream *stream, ALuint buffer);
