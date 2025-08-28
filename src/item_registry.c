#include <stdio.h>
#include <stdlib.h>
#include "item_registry.h"

void item_registry_init(struct ItemRegistry *item_registry, int num_items){
  item_registry->items = (struct ItemDefinition *)calloc(num_items, sizeof(struct ItemDefinition));
  if (!item_registry->items){
    fprintf(stderr, "Error: failed to allocate items in item_registry_init\n");
    return;
  }
  item_registry->num_items = num_items;
}

struct ItemDefinition *item_registry_get_item(struct ItemRegistry *item_registry, int item_id){
  for (unsigned int i = 0; i < item_registry->num_items; i++){
    if (item_registry->items[i].id == item_id){
      return &item_registry->items[i];
    }
  }
  printf("Failed to get item from item registry\n");
  return NULL;
}
