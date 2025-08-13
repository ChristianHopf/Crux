#include "entity.h"

void entity_play_sound_effect(struct Entity *entity){
  if (entity->audio_component->sound_effect_index >= 0){
    audio_component_play(entity->audio_component);
  }
}
