#pragma once

#include <stdbool.h>
#include <uuid/uuid.h>
// #include "scene.h"
// #include "entity.h"
#include "item.h"
#include "item_registry.h"

struct InventoryComponent {
  uuid_t entity_id;
  struct ItemComponent *items;
  int size;
  int capacity;
};

bool inventory_add_item(struct InventoryComponent *inventory_component, struct ItemRegistry *item_registry, int item_id, int count);

void inventory_print(struct ItemRegistry *item_registry, struct InventoryComponent *inventory_component);
