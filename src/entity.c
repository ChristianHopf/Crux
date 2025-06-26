#include "entity.h"

void entity_play_sound_effect(struct Entity *entity, struct SoundEffect *sound_effect){
  if (!alcGetCurrentContext()) {
    alcMakeContextCurrent(audio_context); // Assume audio_context is accessible
    if (!alcGetCurrentContext()) {
      fprintf(stderr, "Error: failed to set OpenAL context\n");
      return;
    }
  }

  // Log inputs
  fprintf(stderr, "entity_play_sound_effect: audio_source=%u, buffer=%u\n",
          entity->audio_source, sound_effect->buffer);
  printf("hi\n");

  // Validate source
  if (!alIsSource(entity->audio_source)) {
    fprintf(stderr, "Error: entity->audio_source (%u) is not a valid source\n", entity->audio_source);
    // Regenerate source if needed
    alGenSources(1, &entity->audio_source);
    ALenum error = alGetError();
    if (error != AL_NO_ERROR) {
      fprintf(stderr, "Error generating source: %d\n", error);
      return;
    }
    fprintf(stderr, "Generated new source: %u\n", entity->audio_source);
  }

  // Validate buffer
  if (!alIsBuffer(sound_effect->buffer)) {
    fprintf(stderr, "Error: sound_effect->buffer (%u) is not a valid buffer\n", sound_effect->buffer);
    return;
  }


  alSourcei(entity->audio_source, AL_BUFFER, sound_effect->buffer);
  alSourcePlay(entity->audio_source);
}

