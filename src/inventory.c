#include "inventory.h"

bool inventory_add_item(struct Scene *scene, uuid_t player_entity_id, int item_id, int count){
  // Get InventoryComponent with given player's entity_id
  struct InventoryComponent *inventory_component = scene_get_inventory_by_entity_id(scene, player_entity_id);
  if (!inventory_component) return false;

  // Add item
  if (inventory_component->size < inventory_component->capacity){
    struct ItemComponent *item = &inventory_component->items[inventory_component->size];
    item->id = item_id;
    item->count = count;
    inventory_component->size++;
    return true;
  }

  return false;
}

void inventory_print(struct InventoryComponent *inventory_component){
  // Print size and capacity
  printf("\nINVENTORY\n");
  printf("Size: %d\nCapacity: %d\n", inventory_component->size, inventory_component->capacity);

  // Print items
  for (unsigned int i = 0; i < inventory_component->size; i++){
    struct ItemComponent *item = &inventory_component->items[i];
    printf("Item %d:\n", i);
    printf("id: %d\n", item->id);
    printf("count: %d\n", item->count);
  }
}
