#include "inventory.h"

bool inventory_add_item(struct Scene *scene, uuid_t player_entity_id, int item_id, int count){
  // Find InventoryComponent with given entity id
  for (unsigned int i = 0; i < scene->num_inventory_components; i++){
    struct InventoryComponent *inventory_component = &scene->inventory_components[i];
    if (uuid_compare(inventory_component->entity_id, player_entity_id) == 0){
      // Add item
      if (inventory_component->size < inventory_component->capacity){
        struct ItemComponent *item = &inventory_component->items[inventory_component->size];
        item->id = item_id;
        item->count = count;
        inventory_component->size++;
        return true;
      }
    }
  }

  // Add item
  return false;
}

void inventory_print(struct InventoryComponent *inventory_component){
  // Print size and capacity
  printf("Inventory size: %d\nInventory capacity: %d\n", inventory_component->size, inventory_component->capacity);

  // Print items
  for (unsigned int i = 0; i < inventory_component->size; i++){
    struct ItemComponent *item = &inventory_component->items[i];
    printf("Item %d:\n", i);
    printf("id: %d\n", item->id);
    printf("count: %d\n", item->count);
  }
}
