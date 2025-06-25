#include "audio_manager.h"
#include <AL/al.h>

struct AudioStream *audio_stream_create(char *path){

  // Allocate AudioStream
  struct AudioStream *stream = calloc(1, sizeof(struct AudioStream));
  if (!stream){
    fprintf(stderr, "Error: failed to allocate AudioStream in audio_stream_create\n");
    return NULL;
  }

  printf("audio path: %s\n", path);
  // Open audio file with sndfile
  stream->file = sf_open(path, SFM_READ, &stream->info);
  if (!stream->file){
    fprintf(stderr, "Error: failed to open %s: %s\n", path, sf_strerror(NULL));
    free(stream);
    return 0;
  }

  // Get format
  if (stream->info.channels == 1){
    stream->format = AL_FORMAT_MONO_FLOAT32;
  }
  else{
    stream->format = AL_FORMAT_STEREO_FLOAT32;
  }

  // Generate and load buffers
  alGenBuffers(NUM_BUFFERS, stream->buffers);
  for(int i = 0; i < NUM_BUFFERS; i++){
    if (!fill_buffer(stream, stream->buffers[i])){
      break;
    }
  }

  // Generate source
  alGenSources(1, &stream->source);
  alSourcef(stream->source, AL_GAIN, 1.0f);
  alSourcef(stream->source, AL_PITCH, 1.0f);

  // Queue buffers
  alSourceQueueBuffers(stream->source, NUM_BUFFERS, stream->buffers);
  if (alGetError() != AL_NO_ERROR){
    fprintf(stderr, "Error: alGetError returned error %d in alSourceQueueBuffers\n", alGetError());
  }

  // Start audio thread
  if (thrd_create(&stream->audio_thread, audio_stream_update, stream) != thrd_success){
    fprintf(stderr, "Error: failed to start audio thread in audio_stream_create\n");
    alDeleteSources(1, &stream->source);
    alDeleteBuffers(NUM_BUFFERS, stream->buffers);
    sf_close(stream->file);
    free(stream);
    return NULL;
  }

  alSourcePlay(stream->source);

  return stream;
}

void audio_stream_destroy(struct AudioStream *stream){

  // Stop stream and join thread
  stream->stop_audio = true;
  thrd_join(stream->audio_thread, NULL);

  // OpenAL cleanup
  alSourceStop(stream->source);
  alDeleteSources(1, &stream->source);
  alDeleteBuffers(NUM_BUFFERS, stream->buffers);

  sf_close(stream->file);
  free(stream);
}

void *audio_stream_update(void *arg){
  struct AudioStream *stream = (struct AudioStream *)arg;
  // alcMakeContextCurrent(context);
  while (!stream->stop_audio){

    // Check for processed buffers
    ALint processed = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);
    if (processed <= 0) continue;

    // For each buffer that's been processed,
    // - unqueue a buffer
    // - load new data
    // - requeue
    for(int i = 0; i < processed; i++){
      ALuint buffer;
      alSourceUnqueueBuffers(stream->source, 1, &buffer);

      if (fill_buffer(stream, buffer)){
        alSourceQueueBuffers(stream->source, 1, &buffer);
      }
    }

    // Check for buffer underrun
    ALint state;
    alGetSourcei(stream->source, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING){
      ALint queued;
      alGetSourcei(stream->source, AL_BUFFERS_QUEUED, &queued);
      if (queued > 0){
        alSourcePlay(stream->source);
      }
    }

    // Sleep for 1 ms
    usleep(1000);
  }
  // alcMakeContextCurrent(NULL);
  return NULL;
}

// HELPERS
bool fill_buffer(struct AudioStream *stream, ALuint buffer){
  float *new_data = malloc(4096 * stream->info.channels * sizeof(float));
  sf_count_t read_frames = sf_readf_float(stream->file, new_data, 4096);

  // End of file, move to beginning of file first
  if (read_frames == 0){
    sf_seek(stream->file, 0, SEEK_SET);
    read_frames = sf_readf_float(stream->file, new_data, 4096);
  }
  // Buffer data normally
  if (read_frames > 0) {
    size_t data_size = read_frames * stream->info.channels * sizeof(float);
    alBufferData(buffer, stream->format, new_data, data_size, stream->info.samplerate);
    free(new_data);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR){
      fprintf(stderr, "Error: failed to load data with alBufferData in audio_stream_update: %d\n", error);
      return false;
    }

    return true;
  }
}
