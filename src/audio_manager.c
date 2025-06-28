#include "audio_manager.h"
#include "player.h"
#include <AL/al.h>
#include <locale.h>
#include <time.h>

ALCdevice *audio_device = NULL;
ALCcontext *audio_context = NULL;

struct SoundEffect sound_effects[MAX_SOUND_EFFECTS];
int num_sound_effects = 0;

void audio_pause(){

}

void audio_unpause(){

}

struct AudioStream *audio_stream_create(char *path){

  alcMakeContextCurrent(audio_context);

  // Allocate AudioStream
  struct AudioStream *stream = calloc(1, sizeof(struct AudioStream));
  if (!stream){
    fprintf(stderr, "Error: failed to allocate AudioStream in audio_stream_create\n");
    return NULL;
  }

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

  // Allocate temp buffer
  stream->temp_buffer = malloc(BUFFER_FRAMES * stream->info.channels * sizeof(float));
  if (!stream->temp_buffer){
    fprintf(stderr, "Error: failed to allocate temp_buffer in audio_stream_create\n");
    sf_close(stream->file);
    free(stream);
    return NULL;
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

int audio_stream_update(void *arg){
  struct AudioStream *stream = (struct AudioStream *)arg;
  alcMakeContextCurrent(audio_context);
  while (!stream->stop_audio){


    // Check for processed buffers
    ALint processed = 0;
    alGetSourcei(stream->source, AL_BUFFERS_PROCESSED, &processed);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
      fprintf(stderr, "Error checking processed buffers: %d\n", error);
      return -1;
    }
    if (processed <= 0) continue;

    // For each buffer that's been processed,
    // - unqueue a buffer
    // - load new data
    // - requeue
    for(int i = 0; i < processed; i++){
      ALuint buffer;
      alSourceUnqueueBuffers(stream->source, 1, &buffer);
      error = alGetError();
      if (error != AL_NO_ERROR) {
        fprintf(stderr, "Error unqueuing buffer: %d\n", error);
        return -1;
      }

      if (fill_buffer(stream, buffer)){
        alSourceQueueBuffers(stream->source, 1, &buffer);
        error = alGetError();
        if (error != AL_NO_ERROR) {
          fprintf(stderr, "Error requeuing buffer %u: %d\n", buffer, error);
          return -1;
        }
      }
      else {
        fprintf(stderr, "Error: fill_buffer failed for buffer %u\n", buffer);
        // Optionally continue instead of exiting
        return -1;
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

    // Sleep for 5 ms
    const struct timespec req = {0, 5000000};
    const struct timespec rem;
    nanosleep(&req, &rem);
  }
  alcMakeContextCurrent(NULL);
  return 0;
}

bool fill_buffer(struct AudioStream *stream, ALuint buffer){
  // float *new_data = malloc(BUFFER_FRAMES * stream->info.channels * sizeof(float));
  sf_count_t read_frames = sf_readf_float(stream->file, stream->temp_buffer, BUFFER_FRAMES);

  // End of file, move to beginning of file first
  if (read_frames == 0){
    sf_seek(stream->file, 0, SEEK_SET);
    read_frames = sf_readf_float(stream->file, stream->temp_buffer, BUFFER_FRAMES);
  }
  // Buffer data normally
  if (read_frames > 0) {
    size_t data_size = read_frames * stream->info.channels * sizeof(float);
    alBufferData(buffer, stream->format, stream->temp_buffer, data_size, stream->info.samplerate);

    ALenum error = alGetError();
    if (error != AL_NO_ERROR){
      fprintf(stderr, "Error: failed to load data with alBufferData in audio_stream_update: %d\n", error);
      return false;
    }

    return true;
  }

  // Still read 0 frames after moving to beginning of file. Probably won't happen
  return false;
}

void audio_sound_effect_create(char *path, char *name){
  // Open sound effect file
  SF_INFO sfx_info;
  SNDFILE *sfx_file = sf_open(path, SFM_READ, &sfx_info);
  if (!sfx_file){
    fprintf(stderr, "Error: failed to open %s: %s\n", path, sf_strerror(NULL));
    return;
  }

  // Get format
  ALenum format;
  if (sfx_info.channels == 1){
    format = AL_FORMAT_MONO_FLOAT32;
  }
  else{
    format = AL_FORMAT_STEREO_FLOAT32;
  }

  // Load and buffer data, add to sound_effects
  float *sfx_data = malloc(sfx_info.frames * sfx_info.channels * sizeof(float));
  sf_count_t read_frames = sf_readf_float(sfx_file, sfx_data, sfx_info.frames);
  ALuint sfx_buffer;
  alGenBuffers(1, &sfx_buffer);
  alBufferData(sfx_buffer, format, sfx_data, sfx_info.frames * sfx_info.channels * sizeof(float), sfx_info.samplerate);
  ALenum sfx_error = alGetError();
  if (sfx_error != AL_NO_ERROR){
    fprintf(stderr, "Error buffering vine boom data: %d\n", sfx_error);
  }
  struct SoundEffect sound_effect = {
    name,
    sfx_buffer
  };
  sound_effects[num_sound_effects++] = sound_effect;

  free(sfx_data);
}

void audio_sound_effect_play(struct SoundEffect *sound_effect){
  ALuint source;
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, sound_effect->buffer);
  alSourcePlay(source);
  // alDeleteSources(1, &source);
}

void audio_listener_update(struct Player *player){
  // Update listener position and orientation
  struct Camera *camera = player->camera;

  alcMakeContextCurrent(audio_context);
  ALenum player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set audio context in player_init\n");
  }
  alListenerfv(AL_POSITION, camera->position);
  player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set audio context in player_init\n");
  }

  // Set listener orientation
  float orientation[6];
  orientation[0] = camera->front[0];
  orientation[1] = camera->front[1];
  orientation[2] = camera->front[2];
  orientation[3] = camera->up[0];
  orientation[4] = camera->up[1];
  orientation[5] = camera->up[2];

  alListenerfv(AL_ORIENTATION, orientation);
  player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set audio context in player_init\n");
  }
}

void game_state_changed(void *instance, GameState *game_state){
  // Check instance
  // Handle game state (pause)
}
