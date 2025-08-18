#include "inventory.h"
#include <stdio.h>

// TODO: pass item entity id, allow player to pick up a partial amount of this item
bool inventory_add_item(struct InventoryComponent *inventory_component, struct ItemRegistry *item_registry, int item_id, int count){
  printf("Time to add a new item with id %d and count %d\n", item_id, count);
  // Get InventoryComponent with given player's entity_id
  // struct InventoryComponent *inventory_component = scene_get_inventory_by_entity_id(scene, player_entity_id);
  // if (!inventory_component) return false;

  struct ItemDefinition *item_definition = item_registry_get_item(item_registry, item_id);

  // If the inventory already has an item with this ID,
  // try to increment its count
  for (unsigned int i = 0; i < inventory_component->size; i++){
    struct ItemComponent *item_component = &inventory_component->items[i];
    if (item_component->id == item_id){
      if (item_component->count + count <= item_definition->max_count){
        item_component->count += count;
        return true;
      }
    }
  }

  // Else, add a new item
  if (inventory_component->size < inventory_component->capacity){
    struct ItemComponent *item = &inventory_component->items[inventory_component->size];
    item->id = item_id;
    item->count = count;
    inventory_component->size++;
    return true;
  }

  return false;
}

void inventory_print(struct ItemRegistry *item_registry, struct InventoryComponent *inventory_component){
  // Print size and capacity
  printf("\nINVENTORY\n");
  printf("Size: %d\nCapacity: %d\n", inventory_component->size, inventory_component->capacity);

  // Print items
  for (unsigned int i = 0; i < inventory_component->size; i++){
    struct ItemComponent *item = &inventory_component->items[i];
    struct ItemDefinition *item_definition = item_registry_get_item(item_registry, item->id);
    printf("Item %d:\n", i);
    printf("ID: %d\n", item->id);
    printf("Name: %s\n", item_definition->name);
    printf("Count: %d\n", item->count);
    printf("Max count: %d\n", item_definition->max_count);
  }
}
