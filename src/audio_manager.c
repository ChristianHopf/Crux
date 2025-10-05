#include "string.h"
#include "scene.h"
#include "audio_manager.h"
#include "camera.h"
#include "game_state.h"
#include "entity.h"
// #include <AL/al.h>
#include <locale.h>
#include <time.h>

bool audio_manager_init(struct AudioManager *audio_manager){
  // global_audio_manager = (struct AudioManager *)calloc(1, sizeof(struct AudioManager));
  // if (!global_audio_manager){
  //   fprintf(stderr, "Error: failed to allocate AudioManager in audio_manager_init\n");
  //   return;
  // }

  // Device
  audio_manager->device = alcOpenDevice(NULL);
  ALenum error;
  if (!audio_manager->device){
    ALCenum error = alcGetError(NULL);
    fprintf(stderr, "Error: failed to open OpenAL device: %d\\n", error);
    return false;
  }

  // Context
  audio_manager->context = alcCreateContext(audio_manager->device, NULL);
  if (!audio_manager->context){
    fprintf(stderr, "Error: failed to create OpenAL context\n");
    alcCloseDevice(audio_manager->device);
    return false;
  }
  alcMakeContextCurrent(audio_manager->context);

  // Sources
  alGenSources(MAX_SOURCES, audio_manager->sources);
  error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to generate source pool in audio_manager_init: %d\n", error);
  }
  for (int i = 0; i < MAX_SOURCES; i++){
    audio_manager->free_sources[i] = true;
  }
  audio_manager->num_active_sources = 0;

  // Global configuration
  alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);

  // Sound effects
  audio_manager->num_sound_effects = 0;

  // Options
  audio_manager->paused = false;

  return true;
}

void audio_manager_destroy(struct AudioManager *audio_manager){
  if (audio_manager){
    if (audio_manager->audio_stream){
      audio_stream_destroy(audio_manager->audio_stream);
    }
    alcDestroyContext(audio_manager->context);
    alcCloseDevice(audio_manager->device);
  }
}

// struct AudioManager *audio_manager_get_global(){
//   return audio_manager;
// }

bool audio_source_pool_get_source(struct AudioManager *audio_manager, ALuint *source){
  // Find free source, update bool array and return it
  for (int i = 0; i < MAX_SOURCES; i++){
    if (audio_manager->free_sources[i]){
      audio_manager->free_sources[i] = false;
      audio_manager->num_active_sources++;
      *source = audio_manager->sources[i];
      return true;
    }
  }
  return false;

  // // Check if active_sources is full
  // if (audio_manager->num_active_sources >= MAX_SOURCES){
  //   return false;
  // }
  //
  // // Add source and increment num_active_sources
  // audio_manager->sources[audio_manager->num_active_sources] = source;
  // audio_manager->num_active_sources++;
  // return true;
}

void audio_source_pool_return_source(struct AudioManager *audio_manager, ALuint source){
  // Find source in sources array
  for (int i = 0; i < MAX_SOURCES; i++){
    if (audio_manager->sources[i] == source && !audio_manager->free_sources[i]){
      // Stop source and detach buffer
      alSourceStop(source);
      alSourcei(source, AL_BUFFER, 0);
      // Mark source as free and decrement active soruces
      audio_manager->free_sources[i] = true;
      audio_manager->num_active_sources--;
    }
  }

  // // Check if source is in sources
  // for (int i = 0; i < audio_manager->num_active_sources; i++){
  //   if (audio_manager->sources[i] == source){
  //     // Remove source and decrement num_active_sources
  //     audio_manager->sources[i] = audio_manager->sources[audio_manager->num_active_sources - 1];
  //     audio_manager->num_active_sources--;
  //     return true;
  //   }
  // }
  // return false;
}

void audio_pause(struct AudioManager *audio_manager){
  // If already paused, exit
  if (audio_manager->paused){
    return;
  }

  // Pause music stream
  ALint state;
  if (audio_manager->audio_stream){
    alGetSourcei(audio_manager->audio_stream->source, AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING){
      alSourcePause(audio_manager->audio_stream->source);
    }
  }
  ALenum error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to get music stream state or pause music stream: %d\n", error);
  }

  // Pause all playing sources
  for(int i = 0; i < audio_manager->num_active_sources; i++){
    alGetSourcei(audio_manager->sources[i], AL_SOURCE_STATE, &state);
    if (state == AL_PLAYING){
      alSourcePause(audio_manager->sources[i]);
    }
    error = alGetError();
    if (error != AL_NO_ERROR){
      fprintf(stderr, "Error: failed to get source state or pause source in audio_pause: %d\n", error);
    }
  }

  audio_manager->paused = true;
}

void audio_unpause(struct AudioManager *audio_manager){
  // If already unpaused, exit
  if (!audio_manager->paused){
    return;
  }

  // Unpause music stream
  ALint state;
  if (audio_manager->audio_stream){
    alGetSourcei(audio_manager->audio_stream->source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED){
      alSourcePlay(audio_manager->audio_stream->source);
    }
  }
  ALenum error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to get music stream state or unpause music stream: %d\n", error);
  }

  // Pause all playing sources
  for(int i = 0; i < audio_manager->num_active_sources; i++){
    alGetSourcei(audio_manager->sources[i], AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED){
      alSourcePlay(audio_manager->sources[i]);
    }
    error = alGetError();
    if (error != AL_NO_ERROR){
      fprintf(stderr, "Error: failed to get source state or unpause source in audio_unpause: %d\n", error);
    }
  }

  audio_manager->paused = false;
}

void audio_stream_create(struct AudioManager *audio_manager, char *path){

  alcMakeContextCurrent(audio_manager->context);

  // Allocate AudioStream
  struct AudioStream *stream = calloc(1, sizeof(struct AudioStream));
  if (!stream){
    fprintf(stderr, "Error: failed to allocate AudioStream in audio_stream_create\n");
    return;
  }

  // Open audio file with sndfile
  stream->file = sf_open(path, SFM_READ, &stream->info);
  if (!stream->file){
    fprintf(stderr, "Error: failed to open %s: %s\n", path, sf_strerror(NULL));
    free(stream);
    return;
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
    return;
  }

  // Generate and load buffers
  alGenBuffers(NUM_BUFFERS, stream->buffers);
  ALenum stream_error = alGetError();
  if (stream_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to generate buffers in audio_stream_create\n");
    return;
  }
  for(int i = 0; i < NUM_BUFFERS; i++){
    if (!fill_buffer(stream, stream->buffers[i])){
      break;
    }
  }

  // Get source from pool
  // ALuint source;
  if (!audio_source_pool_get_source(audio_manager, &stream->source)){
    return;
  }

  // alGenSources(1, &stream->source);
  // stream_error = alGetError();
  // if (stream_error != AL_NO_ERROR){
  //   fprintf(stderr, "Error: failed to generate source in audio_stream_create\n");
  //   return;
  // }
  // if (!audio_add_source(audio_manager, stream->source)){
  //   fprintf(stderr, "Error: failed to add source to source pool in audio_stream_create\n");
  //   alDeleteSources(1, &stream->source);
  //   return;
  // }

  alSourcef(stream->source, AL_GAIN, 1.0f);
  alSourcef(stream->source, AL_PITCH, 1.0f);

  // Queue buffers
  alSourceQueueBuffers(stream->source, NUM_BUFFERS, stream->buffers);
  stream_error = alGetError();
  if (stream_error != AL_NO_ERROR){
    fprintf(stderr, "Error: alGetError returned error %d in alSourceQueueBuffers\n", stream_error);
  }

  // Start audio thread
  if (thrd_create(&stream->audio_thread, audio_stream_update, audio_manager) != thrd_success){
    fprintf(stderr, "Error: failed to start audio thread in audio_stream_create\n");
    alDeleteSources(1, &stream->source);
    alDeleteBuffers(NUM_BUFFERS, stream->buffers);
    sf_close(stream->file);
    free(stream);
    return;
  }

  alSourcePlay(stream->source);

  audio_manager->audio_stream = stream;
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
  struct AudioManager *audio_manager = (struct AudioManager *)arg;
  struct AudioStream *stream = audio_manager->audio_stream;

  alcMakeContextCurrent(audio_manager->context);
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
     struct timespec rem;
    nanosleep(&req, &rem);
  }
  alcMakeContextCurrent(NULL);
  return 0;
}

bool fill_buffer(struct AudioStream *stream, ALuint buffer){
  // Attempt to read frames from the file at its current position
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

void audio_sound_effect_create(struct AudioManager *audio_manager, char *path, char *name){
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
  audio_manager->sound_effects[audio_manager->num_sound_effects++] = sound_effect;

  free(sfx_data);
}

void audio_sound_effect_play(struct SoundEffect *sound_effect){
  ALuint source;
  alGenSources(1, &source);
  alSourcei(source, AL_BUFFER, sound_effect->buffer);
  alSourcePlay(source);
}

void audio_component_create(struct Scene *scene, uuid_t entity_id, struct AudioManager *audio_manager, int sound_effect_index){
  // Reallocate AudioComponent array if full
  if (scene->num_audio_components >= scene->max_audio_components){
    printf("audio component realloc time\n");
    // printf("scene->num_audio_components: %d\n", scene->num_audio_components);
    scene->max_audio_components *= 2;
    scene->audio_components = realloc(scene->audio_components, scene->max_audio_components * sizeof(struct AudioComponent));
  }

  // Initialize AudioComponent
  struct AudioComponent *audio_component = &scene->audio_components[scene->num_audio_components++];
  memcpy(audio_component->entity_id, entity_id, 16);
  
  // alGenSources(1, &audio_component->source_id);
  // ALenum audio_component_error = alGetError();
  // if (audio_component_error != AL_NO_ERROR){
  //   fprintf(stderr, "Error generating AudioComponent source in audio_component_create: %d\n", audio_component_error);
  //   return;
  // }
  // if (!audio_add_source(audio_manager, audio_component->source_id)){
  //   fprintf(stderr, "Error: failed to add source to source pool in audio_component_create\n");
  //   alDeleteSources(1, &audio_component->source_id);
  //   return;
  // }

  // Set source position and options for spatial audio
  // Maybe pass position as an argument?
  // struct Entity *entity = scene_get_entity_by_entity_id(scene, entity_id);
  // if (!entity){
  //   fprintf(stderr, "Error: failed to fetch entity in audio_component_create\n");
  //   return;
  // }
  // alSource3f(audio_component->source_id, AL_POSITION, entity->position[0], entity->position[1], entity->position[2]);
  // ALenum audio_component_error = alGetError();
  // if (audio_component_error != AL_NO_ERROR){
  //   fprintf(stderr, "Error setting AudioComponent source position in audio_component_create: %d\n", audio_component_error);
  //   return;
  // }
  // alSourcef(audio_component->source_id, AL_REFERENCE_DISTANCE, 5.0f);
  // alSourcef(audio_component->source_id, AL_MAX_DISTANCE, 50.0f);
  // alSourcef(audio_component->source_id, AL_ROLLOFF_FACTOR, 1.0f);
  // alSourcei(audio_component->source_id, AL_SOURCE_RELATIVE, AL_FALSE);
  // audio_component_error = alGetError();
  // if (audio_component_error != AL_NO_ERROR){
  //   fprintf(stderr, "Error setting AudioComponent source spatial audio options in audio_component_create: %d\n", audio_component_error);
  //   return;
  // }

  // Assign buffer from sound effects
  // alSourcei(audio_component->source_id, AL_BUFFER, audio_manager->sound_effects[sound_effect_index].buffer);
  // audio_component_error = alGetError();
  // if (audio_component_error != AL_NO_ERROR){
  //   fprintf(stderr, "Error setting AudioComponent buffer in audio_component_create: %d\n", audio_component_error);
  //   return;
  // }
}

void audio_component_update(struct AudioManager *audio_manager, struct AudioComponent *audio_component){
  // Return inactive sources to source pool, update positions
  for (unsigned int i = 0; i < audio_component->num_active_sources; i++){
    ALint state;
    alGetSourcei(audio_component->sources[i], AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING){
      // Return to pool
      audio_source_pool_return_source(audio_manager, audio_component->sources[i]);
      // Swap and pop sources (will have to shift down if/when order matters)
      audio_component->sources[i] = audio_component->sources[audio_component->num_active_sources - 1];
      audio_component->num_active_sources--;
    }
  }

  // Update position (remove from other entity update function)
}

void audio_component_play(struct AudioManager *audio_manager, struct AudioComponent *audio_component, int sound_effect_index){
  // Check for valid sound effect index
  if (sound_effect_index < 0 || sound_effect_index >= audio_manager->num_sound_effects){
    fprintf(stderr, "Error: invalid sound_effect_index %d\n", sound_effect_index);
    return;
  }

  // Check for space to add a new source
  if (audio_component->num_active_sources >= MAX_COMPONENT_SOURCES){
    fprintf(stderr, "Error: audio component is at max sources\n");
    return;
  }

  // Set context if not set
  if (!alcGetCurrentContext()) {
    alcMakeContextCurrent(audio_manager->context);
    if (!alcGetCurrentContext()) {
      fprintf(stderr, "Error: failed to set OpenAL context\n");
      return;
    }
  }

  // Get source from the audio manager's source pool,
  // then set its position and add to audio component's sources
  ALuint source;
  if (!audio_source_pool_get_source(audio_manager, &source)){
    return;
  }
  // ALuint source = audio_manager->sources[audio_manager->num_active_sources++];
  alSourcei(source, AL_BUFFER, audio_manager->sound_effects[sound_effect_index].buffer);
  ALenum error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error setting AudioComponent buffer in audio_component_play: %d\n", error);
    return;
  }

  alSource3f(source, AL_POSITION, audio_component->position[0], audio_component->position[1], audio_component->position[2]);
  error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error setting source position in audio_component_play: %d\n", error);
    return;
  }

  audio_component->sources[audio_component->num_active_sources++] = source;

  // Get current state of the AudioComponent's source
  // ALint state;
  // alGetSourcei(audio_component->source_id, AL_SOURCE_STATE, &state);
  // ALenum error = alGetError();
  // if (error != AL_NO_ERROR) {
  //   fprintf(stderr, "Error checking entity->audio_source state: %d\n", error);
  //   return;
  // }
  //
  // // If it's already playing, stop it
  // // - later implement generating new sources for overlapping sound effects from the same entity
  // if (state == AL_PLAYING){
  //   alSourceStop(audio_component->source_id);
  //   error = alGetError();
  //   if (error != AL_NO_ERROR){
  //     fprintf(stderr, "Error stopping entity->audio_source: %d\n", error);
  //     return;
  //   }
  // }

  // This is no longer needed while an AudioComponent can only have one sound effect: assume its source already uses the right buffer
  //
  // alSourcei(audio_component->source_id, AL_BUFFER, audio_manager->sound_effects[audio_component->sound_effect_index]);
  // error = alGetError();
  // if (error != AL_NO_ERROR){
  //   fprintf(stderr, "Error assigning sound effect buffer to entity->audio_source %d\n", error);
  //   return;
  // }

  // Assign desired sound effect buffer to source
  // alSourcei(audio_component->source_id, AL_BUFFER, audio_manager->sound_effects[sound_effect_index].buffer);
  // error = alGetError();
  // if (error != AL_NO_ERROR){
  //   fprintf(stderr, "Error setting AudioComponent buffer in audio_component_create: %d\n", error);
  //   return;
  // }

  // Play the sound
  alSourcePlay(source);
  error = alGetError();
  if (error != AL_NO_ERROR){
    fprintf(stderr, "Error playing sound effect: %d\n", error);
    return;
  }
}

void audio_component_destroy(struct AudioManager *audio_manager, struct AudioComponent *audio_component){
  // Return all sources to source pool
  for (int i = 0; i < audio_component->num_active_sources; i++){
    audio_source_pool_return_source(audio_manager, audio_component->sources[i]);
  }

  // if (!audio_remove_source(audio_manager, audio_component->source_id)){
  //   fprintf(stderr, "Error: failed to remove audio source in audio_component_destroy\n");
  // }
}

void audio_listener_update(struct Scene *scene, uuid_t entity_id){
  struct CameraComponent *camera = scene_get_camera_by_entity_id(scene, entity_id);

  // Set context
  // alcMakeContextCurrent(audio_manager->context);
  ALenum player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set audio context in audio_listener_update\n");
  }

  // Set listener position
  alListenerfv(AL_POSITION, camera->position);
  player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set listener position in audio_listener_update\n");
  }

  // Get listener up vector
  vec3 listener_up;
  glm_cross(camera->front, camera->right, listener_up);
  glm_normalize(listener_up);

  // Set listener orientation
  // (Need to negate camera->front for some reason. I thought OpenGL rendering was right-handed?)
  float orientation[6];
  orientation[0] = -camera->front[0];
  orientation[1] = -camera->front[1];
  orientation[2] = -camera->front[2];
  orientation[3] = listener_up[0];
  orientation[4] = listener_up[1];
  orientation[5] = listener_up[2];

  alListenerfv(AL_ORIENTATION, orientation);
  player_error = alGetError();
  if (player_error != AL_NO_ERROR){
    fprintf(stderr, "Error: failed to set audio context in player_init\n");
  }
}


struct GameStateObserver *audio_game_state_observer_create(struct AudioManager *audio_manager){
  struct GameStateObserver *audio_game_state_observer = (struct GameStateObserver *)malloc(sizeof(struct GameStateObserver));
  if (!audio_game_state_observer){
    fprintf(stderr, "Error: failed to allocate audio_game_state_observer in engine_create\n");
    return NULL;
  }

  audio_game_state_observer->instance = audio_manager;
  audio_game_state_observer->notification = audio_game_state_changed;

  return audio_game_state_observer;
}

void audio_game_state_changed(void *instance, GameState *game_state){
  // Check instance (some kind of observer type enum, just make it work for now)
  struct AudioManager *audio_manager = (struct AudioManager *)instance;

  // Handle game state (pause)
  if (game_state->is_paused){
    audio_pause(audio_manager);
  }
  else{
    audio_unpause(audio_manager);
  }
}
