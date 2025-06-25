#pragma once

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <sndfile.h>
#include <stdlib.h>
#include <stdbool.h>
#include "tinycthread/tinycthread.h"

#define NUM_BUFFERS 4

struct AudioStream {
  SNDFILE *file;
  SF_INFO info;
  ALenum format;
  ALuint buffers[NUM_BUFFERS];
  ALuint source;

  thrd_t audio_thread;

  bool stop_audio;
};

// Audio context from main
extern ALCcontext *audio_context;

struct AudioStream *audio_stream_create(char *path);
void audio_stream_destroy(struct AudioStream *stream);
void *audio_stream_update(void *arg);

// HELPERS
bool fill_buffer(struct AudioStream *stream, ALuint buffer);
