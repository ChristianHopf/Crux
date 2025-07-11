#include "entity.h"

void entity_play_sound_effect(struct Entity *entity){
  if (entity->audio_component->sound_effect_index >= 0){
    audio_component_play(entity->audio_component);
  }

  // if (!alcGetCurrentContext()) {
  //   alcMakeContextCurrent(audio_manager->context);
  //   if (!alcGetCurrentContext()) {
  //     fprintf(stderr, "Error: failed to set OpenAL context\n");
  //     return;
  //   }
  // }
  //
  // // Get current state of entity->audio_source
  // ALint state;
  // alGetSourcei(audio_component->source_id, AL_SOURCE_STATE, &state);
  // ALenum error = alGetError();
  // if (error != AL_NO_ERROR) {
  //   fprintf(stderr, "Error checking entity->audio_source state: %d\n", error);
  //   return;
  // }
  //
  // // If it's already playing, stop it first
  // if (state == AL_PLAYING){
  //   alSourceStop(audio_component->source_id);
  //   error = alGetError();
  //   if (error != AL_NO_ERROR){
  //     fprintf(stderr, "Error stopping entity->audio_source: %d\n", error);
  //     return;
  //   }
  // }
  //
  // // Assign this sound effect's buffer to the entity's audio_source
  // // (To support multiple overlapping sound effects, I'll need to
  // // generate a new source, then keep track of it somehow to delete it
  // // once it's done playing. For now, only one sound effect can play at a time.)
  // alSourcei(audio_component->source_id, AL_BUFFER, audio_manager->sound_effects[audio_component->sound_effect_index]);
  // error = alGetError();
  // if (error != AL_NO_ERROR){
  //   fprintf(stderr, "Error assigning sound effect buffer to entity->audio_source %d\n", error);
  //   return;
  // }
  //
  // alSourcePlay(audio_component->source);
  // error = alGetError();
  // if (error != AL_NO_ERROR){
  //   fprintf(stderr, "Error playing sound effect: %d\n", error);
  //   return;
  // }
}

// void entity_play_sound_effect(struct Entity *entity, struct SoundEffect *sound_effect){
//   if (!alcGetCurrentContext()) {
//     alcMakeContextCurrent(audio_context);
//     if (!alcGetCurrentContext()) {
//       fprintf(stderr, "Error: failed to set OpenAL context\n");
//       return;
//     }
//   }
//
//   // Get current state of entity->audio_source
//   ALint state;
//   alGetSourcei(entity->audio_source, AL_SOURCE_STATE, &state);
//   ALenum error = alGetError();
//   if (error != AL_NO_ERROR) {
//     fprintf(stderr, "Error checking entity->audio_source state: %d\n", error);
//     return;
//   }
//
//   // If it's already playing, stop it first
//   if (state == AL_PLAYING){
//     alSourceStop(entity->audio_source);
//     error = alGetError();
//     if (error != AL_NO_ERROR){
//       fprintf(stderr, "Error stopping entity->audio_source: %d\n", error);
//       return;
//     }
//   }
//
//   // Assign this sound effect's buffer to the entity's audio_source
//   // (To support multiple overlapping sound effects, I'll need to
//   // generate a new source, then keep track of it somehow to delete it
//   // once it's done playing. For now, only one sound effect can play at a time.)
//   alSourcei(entity->audio_source, AL_BUFFER, sound_effect->buffer);
//   error = alGetError();
//   if (error != AL_NO_ERROR){
//     fprintf(stderr, "Error assigning sound effect buffer to entity->audio_source %d\n", error);
//     return;
//   }
//
//   alSourcePlay(entity->audio_source);
//   error = alGetError();
//   if (error != AL_NO_ERROR){
//     fprintf(stderr, "Error playing sound effect: %d\n", error);
//     return;
//   }
// }
