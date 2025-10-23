#include "trigger.h"

void trigger_component_create(struct Scene *scene, uuid_t entity_id, struct AABB aabb){
  struct TriggerComponent *trigger = &scene->trigger_components[scene->num_trigger_components++];
  if (!trigger){
    fprintf(stderr, "Error: failed to get TriggerComponent in trigger_component_create\n");
  }

  memcpy(trigger_component->entity_id, entity_id, 16);
  trigger_component->aabb = aabb;
}

void trigger_component_destroy(struct TriggerComponent *trigger_component){

}


