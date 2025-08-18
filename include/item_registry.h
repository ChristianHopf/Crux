#pragma once

#define MAX_ITEM_NAME_LENGTH 64

struct ItemDefinition {
  int id;
  char name[64];
  int max_count;
};

struct ItemRegistry {
  struct ItemDefinition *items;
  int num_items;
};


void item_registry_init(struct ItemRegistry *item_registry, int num_items);

struct ItemDefinition *item_registry_get_item(struct ItemRegistry *item_registry, int item_id);
