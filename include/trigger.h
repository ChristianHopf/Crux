#pragma once

#include <uuid/uuid.h>
#include <stdbool.h>
#include "physics/aabb.h"
#include "objective.h"
#include "event.h"
#include "engine.h"

typedef enum {
  TRIGGER_EXIT_LEVEL,
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
  struct AABB aabb;
  struct TriggerBehavior behavior;
};


void trigger_component_create(struct Scene *scene, uuid_t entity_id, struct AABB aabb);
void trigger_component_destroy(struct TriggerComponent *trigger_component);
void trigger_component_process_event(struct TriggerComponent *trigger_component, struct GameEvent *game_event);

void trigger_component_add_behavior_exit_level(struct TriggerComponent *trigger_component, bool require_objectives_complete);
