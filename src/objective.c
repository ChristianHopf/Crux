#include "event.h"
#include "objective.h"
#include <stdio.h>
#include <string.h>

bool objective_manager_init(struct ObjectiveManager *objective_manager){
  memset(objective_manager, 0, sizeof(struct ObjectiveManager));

  return true;
}

void objective_manager_update_on_event(struct ObjectiveManager *objective_manager, struct GameEvent *game_event){
  // For each objective, if it isn't complete, switch on its ObjectiveType
  // and check if this event satisfies its conditions
  for (unsigned int i = 0; i < objective_manager->num_objectives; i++){
    struct Objective *objective = &objective_manager->objectives[i];
    if (objective->complete) continue;

    switch (objective->type){
      case OBJECTIVE_COLLECT_ITEM: {
        if (game_event->type == EVENT_PLAYER_ITEM_PICKUP){
          if (game_event->data.item_pickup.item_id == objective->data.collect_item.item_id){
            objective->data.collect_item.current_count += game_event->data.item_pickup.item_count;
          printf("Objective: %s\nRequired item count: %d\nCurrent item count: %d\n\n", objective->description, objective->data.collect_item.required_count, objective->data.collect_item.current_count);
            if (objective->data.collect_item.current_count >= objective->data.collect_item.required_count){
              objective->complete = true;
              printf("Objective complete!\n");
            }
          }
        }
        break;
      }
      default: {
        break;
      }
    }
  }
}

void objective_manager_destroy(struct ObjectiveManager *objective_manager){
}

void objective_manager_objective_add(struct ObjectiveManager *objective_manager, struct Objective objective){
  if (!objective_manager || objective_manager->num_objectives >= MAX_OBJECTIVES){
    fprintf(stderr, "Error: failed to add objective, either ObjectiveManager is invalid or max objectives reached\n");
    return;
  }

  objective_manager->objectives[objective_manager->num_objectives++] = objective;
}

bool objective_manager_all_complete(struct ObjectiveManager *objective_manager){
  for (unsigned int i = 0; i < objective_manager->num_objectives; i++){
    if (!objective_manager->objectives[i].complete){
      return false;
    }
  }
  return true;
}
