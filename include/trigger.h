#pragma once

#include <uuid/uuid.h>
#include <stdbool.h>
#include "physics/aabb.h"
#include "objective.h"
#include "event.h"
#include "engine.h"

typedef enum {
  TRIGGER_EXIT_LEVEL = 0,
  TRIGGER_CUSTOM
} TriggerBehaviorType;

struct TriggerBehavior {
  TriggerBehaviorType type;
  union {
    struct {
      bool require_objectives_complete;
    } exit_level;
    void *custom;
  } data;
};

struct TriggerComponent {
  uuid_t entity_id;
  // struct AABB aabb;
  struct TriggerBehavior trigger_behavior;
};


void trigger_component_create(struct Scene *scene, uuid_t entity_id);
void trigger_component_destroy(struct TriggerComponent *trigger_component);
void trigger_process_event(struct Scene *scene, struct ObjectiveManager *objective_manager, struct GameEvent *game_event);

void trigger_component_add_behavior_exit_level(struct TriggerComponent *trigger_component, bool require_objectives_complete);
