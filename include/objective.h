#pragma once

#include <stdbool.h>

#define MAX_OBJECTIVES 32

typedef enum {
  OBJECTIVE_COLLECT_ITEM
} ObjectiveType;

struct Objective {
  ObjectiveType type;
  char *description;
  union {
    struct {
      int item_id;
      int required_count;
      int current_count;
    } collect_item;
  } data;
  bool complete;
  void *user_data;
};

struct ObjectiveManager {
  struct Objective objectives[MAX_OBJECTIVES];
  unsigned int num_objectives;
};


bool objective_manager_init(struct ObjectiveManager *objective_manager);
void objective_manager_update_on_event(struct ObjectiveManager *objective_manager, struct GameEvent *game_event);
void objective_manager_destroy(struct ObjectiveManager *objective_manager);
void objective_manager_objective_add(struct ObjectiveManager *objective_manager, struct Objective objective);
bool objective_manager_all_complete(struct ObjectiveManager *objective_manager);
