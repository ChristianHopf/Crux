#include "trigger.h"
#include <string.h>
#include "scene.h"

void trigger_component_create(struct Scene *scene, uuid_t entity_id){
  struct TriggerComponent *trigger_component = &scene->trigger_components[scene->num_trigger_components++];
  if (!trigger_component){
    fprintf(stderr, "Error: failed to get TriggerComponent in trigger_component_create\n");
  }

  memcpy(trigger_component->entity_id, entity_id, 16);
  // trigger_component->aabb = aabb;
  // trigger_component->trigger_behavior.type = behavior_type;
}

void trigger_component_destroy(struct TriggerComponent *trigger_component){
  // This doesn't need to do anything yet.
}

void trigger_process_event(struct Scene *scene, struct ObjectiveManager *objective_manager, struct GameEvent *game_event){
  printf("trigger_process_event\n");
  // Get TriggerComponent
  struct TriggerComponent *trigger_component = scene_get_trigger_component_by_entity_id(scene, game_event->data.trigger.trigger_entity_id);

  // Switch on TriggerBehaviorType and process event
  struct TriggerBehavior *trigger_behavior = &trigger_component->trigger_behavior;
  printf("Trigger behavior type is %d\n", trigger_behavior->type);
  switch (trigger_behavior->type){
    case TRIGGER_EXIT_LEVEL:
      if (trigger_behavior->data.exit_level.require_objectives_complete){
        if (objective_manager_all_complete(objective_manager)){
          printf("Objectives complete! Time to exit level\n");
        }
        else{
          printf("There are still objectives to complete, cannot exit level\n");
        }
      }
      break;
    case TRIGGER_CUSTOM:
      break;
  }
}

void trigger_component_add_behavior_exit_level(struct TriggerComponent *trigger_component, bool require_objectives_complete){
  trigger_component->trigger_behavior.type = TRIGGER_EXIT_LEVEL;
  trigger_component->trigger_behavior.data.exit_level.require_objectives_complete = require_objectives_complete;
}
